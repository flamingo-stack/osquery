#!/usr/bin/env python3
"""Release schemas must describe the selected build, including valid SQL aliases."""

import importlib.util
import hashlib
import json
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch
import subprocess


MODULE_PATH = Path(__file__).with_name("release_schema.py")
SPEC = '''table_name("sample", aliases=["sample_alias"])
description("A sample table.")
schema([
    Column("path", TEXT, "Path", aliases=["location"], required=True, optimized=True),
    ForeignKey(column="path", table="file"),
])
extended_schema(WINDOWS, [Column("windows_only", INTEGER, "Windows value")])
notes("Needs permission.")
examples(["select location from sample_alias where path = '/tmp'"])
attributes(cacheable=True)
implementation("sample@genSample")
'''


class ReleaseSchemaTest(unittest.TestCase):
    def setUp(self):
        self.assertTrue(MODULE_PATH.exists(), "release schema exporter is missing")
        spec = importlib.util.spec_from_file_location("release_schema", MODULE_PATH)
        self.exporter = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.exporter)
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "specs").mkdir()
        self.sample = self.root / "specs/sample.table"
        self.sample.write_text(SPEC)
        self.native = self.root / "native.txt"
        self.foreign = self.root / "foreign.txt"
        self.native.write_text(str(self.sample))
        self.foreign.write_text("")

    def variant(self, platform="darwin", architecture="arm64"):
        return self.exporter.export_variant(
            self.root, self.native, self.foreign, platform, architecture,
            "0.0.8", "0.0.8", "a" * 40,
        )

    def test_preserves_aliases_constraints_examples_and_foreign_keys(self):
        table = self.variant()["variants"][0]["tables"][0]
        self.assertEqual(table["aliases"], ["sample_alias"])
        self.assertEqual(table["columns"][0]["aliases"], ["location"])
        self.assertEqual(table["columns"][0]["options"], {"required": True, "optimized": True})
        self.assertEqual(table["foreignKeys"], [{"column": "path", "table": "file"}])
        self.assertEqual(table["notes"], "Needs permission.")
        self.assertEqual(table["examples"], ["select location from sample_alias where path = '/tmp'"])
        self.assertEqual(table["availability"], "native")

    def test_platform_columns_do_not_depend_on_exporter_host(self):
        darwin = self.variant()["variants"][0]["tables"][0]["columns"][1]
        windows = self.variant("windows", "amd64")["variants"][0]["tables"][0]["columns"][1]
        self.assertEqual(darwin["platforms"], ["windows"])
        self.assertTrue(darwin["options"]["hidden"])
        self.assertNotIn("hidden", windows["options"])

    def test_only_cmake_selected_specs_are_exported_and_foreign_is_marked(self):
        (self.root / "specs/unselected.table").write_text(SPEC.replace("sample", "unselected"))
        other = self.root / "specs/other.table"
        other.write_text(SPEC.replace("sample", "other"))
        self.foreign.write_text(str(other))
        tables = self.variant()["variants"][0]["tables"]
        self.assertEqual([(t["name"], t["availability"]) for t in tables],
                         [("other", "foreign"), ("sample", "native")])

    def test_rejects_specs_outside_source_and_empty_native_selection(self):
        self.native.write_text(str(self.root / "outside.table"))
        with self.assertRaises(ValueError):
            self.variant()
        self.native.write_text("")
        with self.assertRaises(ValueError):
            self.variant()

    def test_rejects_duplicate_native_foreign_spec(self):
        self.foreign.write_text(str(self.sample))
        with self.assertRaises(ValueError):
            self.variant()

    def test_aggregation_requires_three_variants_with_matching_provenance(self):
        parts = [self.variant("darwin", "amd64"), self.variant(), self.variant("windows", "amd64")]
        artifact = self.exporter.aggregate(parts)
        self.assertEqual([(v["platform"], v["architecture"]) for v in artifact["variants"]],
                         [("darwin", "amd64"), ("darwin", "arm64"), ("windows", "amd64")])
        self.assertEqual(artifact["sourceCommit"], "a" * 40)
        with self.assertRaises(ValueError):
            self.exporter.aggregate(parts[:2])
        parts[2]["sourceCommit"] = "b" * 40
        with self.assertRaises(ValueError):
            self.exporter.aggregate(parts)

    def test_rejects_duplicate_variant(self):
        parts = [self.variant("darwin", "amd64"), self.variant(), self.variant("windows", "amd64")]
        with self.assertRaises(ValueError):
            self.exporter.aggregate(parts + [parts[0]])

    def test_sidecar_hashes_exact_utf8_bytes_without_reserializing(self):
        first = self.variant()
        first["variants"][0]["tables"][0]["notes"] = "Permissions — needed"
        output = self.root / "osquery-schema.json"
        self.assertTrue(hasattr(self.exporter, "write_artifact"), "raw checksum writer is missing")
        self.exporter.write_artifact(first, output)
        raw = output.read_bytes()
        checksum = output.with_name("osquery-schema.json.sha256").read_text()
        self.assertEqual(checksum, hashlib.sha256(raw).hexdigest() + "  osquery-schema.json\n")
        self.assertNotIn("schemaSha256", first)
        self.assertEqual(json.loads(raw)["variants"][0]["tables"][0]["notes"], "Permissions — needed")
        self.assertEqual(json.loads(json.dumps(first))["formatVersion"], 1)

    def binary_query(self, version="0.0.8", columns=("path", "windows_only"), missing_alias=False):
        def run(command, **kwargs):
            sql = command[-1]
            if sql == "select version from osquery_info;":
                rows = [{"version": version}]
            elif sql.startswith("select name from pragma_table_xinfo("):
                rows = [{"name": name} for name in columns]
            elif sql == 'select [location] from "sample" limit 0;':
                if missing_alias:
                    raise subprocess.CalledProcessError(1, command, stderr="no such column: location")
                rows = []
            else:
                raise AssertionError("Unexpected binary SQL: {}".format(sql))
            return SimpleNamespace(stdout=json.dumps(rows))
        return run

    def test_binary_verification_rejects_wrong_engine_version(self):
        with patch.object(self.exporter.subprocess, "run", side_effect=self.binary_query(version="0.0.7")):
            with self.assertRaisesRegex(ValueError, "version differs"):
                self.exporter.verify_binary(self.variant(), self.root / "osqueryd")

    def test_binary_verification_rejects_missing_table_or_column(self):
        for columns in ((), ("path",)):
            with self.subTest(columns=columns):
                with patch.object(self.exporter.subprocess, "run", side_effect=self.binary_query(columns=columns)):
                    with self.assertRaisesRegex(ValueError, "missing columns"):
                        self.exporter.verify_binary(self.variant(), self.root / "osqueryd")

    def test_binary_verification_rejects_missing_column_alias_using_identifier_sql(self):
        with patch.object(self.exporter.subprocess, "run", side_effect=self.binary_query(missing_alias=True)):
            with self.assertRaises(subprocess.CalledProcessError):
                self.exporter.verify_binary(self.variant(), self.root / "osqueryd")

    def test_binary_verification_does_not_treat_foreign_tables_as_supported(self):
        artifact = self.variant()
        artifact["variants"][0]["tables"][0]["availability"] = "foreign"
        with patch.object(self.exporter.subprocess, "run", side_effect=self.binary_query(columns=())):
            self.exporter.verify_binary(artifact, self.root / "osqueryd")

    def test_binary_verification_accepts_registered_table_and_column_aliases(self):
        with patch.object(self.exporter.subprocess, "run", side_effect=self.binary_query()):
            self.exporter.verify_binary(self.variant(), self.root / "osqueryd")


if __name__ == "__main__":
    unittest.main()
