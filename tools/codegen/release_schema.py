#!/usr/bin/env python3
"""Publish the table specs selected for each released osquery binary."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

import gentable


REQUIRED_VARIANTS = {("darwin", "amd64"), ("darwin", "arm64"), ("windows", "amd64")}
PROVENANCE_FIELDS = ("formatVersion", "releaseTag", "osqueryVersion", "sourceCommit")


def selected_specs(manifest, source_root):
    paths = [Path(value).resolve() for value in manifest.read_text(encoding="utf-8").strip().split(";") if value]
    specs_root = (source_root / "specs").resolve()
    for path in paths:
        if specs_root not in path.parents or path.suffix != ".table" or not path.is_file():
            raise ValueError("Invalid selected spec: {}".format(path))
    if len(set(paths)) != len(paths):
        raise ValueError("Duplicate specs in {}".format(manifest))
    return paths


def platform_names(platforms):
    return sorted({"windows" if name in gentable.WINDOWS else name for name in platforms})


def table_metadata(path, platform, availability, source_root, source_commit):
    gentable.PLATFORM = "win32" if platform == "windows" else platform
    gentable.table.__init__()
    namespace = dict(vars(gentable))
    exec(compile(path.read_text(encoding="utf-8"), str(path), "exec"), namespace)
    table = gentable.table
    if not table.table_name or not table.columns() or not table.function:
        raise ValueError("Incomplete table spec: {}".format(path))
    if gentable.is_denylisted(table.table_name, path=str(path)):
        availability = "disabled"
    return {
        "name": table.table_name,
        "aliases": table.aliases,
        "description": table.description,
        "notes": table.notes,
        "examples": table.examples,
        "attributes": table.attributes,
        "availability": availability,
        "url": "https://github.com/flamingo-stack/osquery/blob/{}/{}".format(
            source_commit, path.relative_to(source_root).as_posix()),
        "columns": [{
            "name": column.name,
            "aliases": column.aliases,
            "type": column.type.affinity.replace("_TYPE", "").lower(),
            "description": column.description,
            "notes": column.notes,
            "platforms": platform_names(column.platforms),
            "options": column.options,
        } for column in table.columns()],
        "foreignKeys": [{"column": key.column, "table": key.table} for key in table.foreign_keys()],
    }


def export_variant(source_root, native_manifest, foreign_manifest, platform, architecture,
                   release_tag, osquery_version, source_commit):
    source_root = Path(source_root).resolve()
    if (platform, architecture) not in REQUIRED_VARIANTS:
        raise ValueError("Unsupported release platform/architecture")
    if not re.fullmatch(r"[0-9a-f]{40}", source_commit):
        raise ValueError("sourceCommit must be the full source commit SHA")
    if not release_tag or not re.fullmatch(r"\d+\.\d+\.\d+", osquery_version):
        raise ValueError("Missing release tag or invalid osquery version")
    native = selected_specs(Path(native_manifest), source_root)
    foreign = selected_specs(Path(foreign_manifest), source_root)
    if not native or set(native) & set(foreign):
        raise ValueError("Native selection is empty or overlaps foreign selection")
    tables = sorted(
        [table_metadata(path, platform, availability, source_root, source_commit)
         for paths, availability in ((native, "native"), (foreign, "foreign")) for path in paths],
        key=lambda table: table["name"],
    )
    if len({table["name"] for table in tables}) != len(tables):
        raise ValueError("Duplicate selected table names")
    variants = [{"platform": platform, "architecture": architecture, "tables": tables}]
    return {"formatVersion": 1, "releaseTag": release_tag, "osqueryVersion": osquery_version,
            "sourceCommit": source_commit, "variants": variants}


def aggregate(parts):
    if not parts:
        raise ValueError("No schema variants")
    provenance = {field: parts[0][field] for field in PROVENANCE_FIELDS}
    variants = []
    for part in parts:
        if any(part.get(field) != value for field, value in provenance.items()):
            raise ValueError("Schema variants have different source/version provenance")
        variants.extend(part["variants"])
    keys = [(variant["platform"], variant["architecture"]) for variant in variants]
    if set(keys) != REQUIRED_VARIANTS or len(keys) != len(REQUIRED_VARIANTS):
        raise ValueError("Release needs exactly darwin/amd64, darwin/arm64 and windows/amd64")
    variants.sort(key=lambda variant: (variant["platform"], variant["architecture"]))
    return dict(provenance, variants=variants)


def write_artifact(artifact, output):
    raw = (json.dumps(artifact, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(raw)
    checksum = hashlib.sha256(raw).hexdigest() + "  " + output.name + "\n"
    output.with_name(output.name + ".sha256").write_text(checksum, encoding="ascii")


def verify_binary(artifact, binary):
    """Check version and SQL column/alias registration without reading table data."""
    with tempfile.TemporaryDirectory(prefix="osquery-schema-") as scratch:
        config = Path(scratch) / "config.json"
        config.write_text("{}", encoding="utf-8")
        def query(sql):
            result = subprocess.run(
                [str(Path(binary).resolve()), "-S", "--json", "--disable_extensions",
                 "--disable_events", "--disable_logging", "--config_plugin=filesystem",
                 "--config_path=" + str(config), "--database_path=" + str(Path(scratch) / "db"), sql],
                capture_output=True, text=True, check=True, timeout=30,
            )
            return json.loads(result.stdout)

        rows = query("select version from osquery_info;")
        if len(rows) != 1 or rows[0]["version"] != artifact["osqueryVersion"]:
            raise ValueError("Built binary version differs from schema version")
        for table in artifact["variants"][0]["tables"]:
            if table["availability"] != "native":
                continue
            all_columns = {column["name"] for column in table["columns"]}
            visible_columns = {column["name"] for column in table["columns"]
                               if not column["options"].get("hidden", False)}
            for name in [table["name"]] + table["aliases"]:
                expected = all_columns if name == table["name"] else visible_columns
                escaped = name.replace("'", "''")
                columns = query("select name from pragma_table_xinfo('{}');".format(escaped))
                names = {column["name"] for column in columns}
                if not expected <= names:
                    raise ValueError("Built table {} missing columns {}".format(name, sorted(expected - names)))
            for column in table["columns"]:
                for alias in column["aliases"]:
                    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", alias):
                        raise ValueError("Unexpected SQL column alias: {}".format(alias))
                    quoted_table = table["name"].replace('"', '""')
                    query('select [{}] from "{}" limit 0;'.format(alias, quoted_table))


def main():
    parser = argparse.ArgumentParser(__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    variant = commands.add_parser("variant")
    variant.add_argument("--source-root", type=Path, default=Path(__file__).resolve().parents[2])
    variant.add_argument("--native-specs", type=Path, required=True)
    variant.add_argument("--foreign-specs", type=Path, required=True)
    variant.add_argument("--platform", choices=("darwin", "windows"), required=True)
    variant.add_argument("--architecture", choices=("amd64", "arm64"), required=True)
    variant.add_argument("--release-tag", required=True)
    variant.add_argument("--osquery-version", required=True)
    variant.add_argument("--source-commit", required=True)
    variant.add_argument("--binary", type=Path)
    variant.add_argument("--output", type=Path, required=True)
    merge = commands.add_parser("aggregate")
    merge.add_argument("--variants", type=Path, nargs="+", required=True)
    merge.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.command == "variant":
        artifact = export_variant(args.source_root, args.native_specs, args.foreign_specs,
                                  args.platform, args.architecture, args.release_tag,
                                  args.osquery_version, args.source_commit)
        if args.binary:
            verify_binary(artifact, args.binary)
    else:
        artifact = aggregate([json.loads(path.read_text(encoding="utf-8")) for path in args.variants])
    write_artifact(artifact, args.output)
    print("Exported {} variant(s), {} tables: {}".format(
        len(artifact["variants"]), sum(len(v["tables"]) for v in artifact["variants"]), args.output))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        print("Schema export failed: {}".format(error), file=sys.stderr)
        sys.exit(1)
