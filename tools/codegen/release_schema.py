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


# Each release contains a catalog for these three binary builds.
REQUIRED_VARIANTS = {("darwin", "amd64"), ("darwin", "arm64"), ("windows", "amd64")}
# Build catalogs can be combined only when they describe the same source and release.
PROVENANCE_FIELDS = ("formatVersion", "releaseTag", "osqueryVersion", "sourceCommit")


def selected_specs(manifest, source_root):
    # CMake lists the .table files selected for this build. Do not scan every spec:
    # doing so could include tables that were not selected for this binary.
    paths = [Path(value).resolve() for value in manifest.read_text(encoding="utf-8").strip().split(";") if value]
    specs_root = (source_root / "specs").resolve()
    for path in paths:
        if specs_root not in path.parents or path.suffix != ".table" or not path.is_file():
            raise ValueError("Invalid selected spec: {}".format(path))
    if len(set(paths)) != len(paths):
        raise ValueError("Duplicate specs in {}".format(manifest))
    return paths


def platform_names(platforms):
    # Use the public platform name in JSON instead of Python's Windows identifiers.
    return sorted({"windows" if name in gentable.WINDOWS else name for name in platforms})


def table_metadata(path, platform, availability, source_root, source_commit):
    # Read each trusted repository .table file with osquery's existing generator.
    # Use the target OS, not the OS running this script, and reset the previous table.
    gentable.PLATFORM = "win32" if platform == "windows" else platform
    gentable.table.__init__()
    namespace = dict(vars(gentable))
    exec(compile(path.read_text(encoding="utf-8"), str(path), "exec"), namespace)
    table = gentable.table
    # Fail on an incomplete table; never publish a partial catalog by skipping it.
    if not table.table_name or not table.columns() or not table.function:
        raise ValueError("Incomplete table spec: {}".format(path))
    if gentable.is_denylisted(table.table_name, path=str(path)):
        availability = "disabled"
    # Copy every column returned by the generator, including hidden columns.
    # Descriptions, aliases and constraints help the AI construct a suitable query.
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
    # Native tables have implementations for this build. Foreign tables describe
    # other platforms; disabled tables are excluded by osquery's denylist.
    native = selected_specs(Path(native_manifest), source_root)
    foreign = selected_specs(Path(foreign_manifest), source_root)
    if not native or set(native) & set(foreign):
        raise ValueError("Native selection is empty or overlaps foreign selection")
    # Process every selected file. A read, parse or conversion error stops export.
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
    # Combine the three build catalogs without mixing releases or source commits.
    if not parts:
        raise ValueError("No schema variants")
    provenance = {field: parts[0][field] for field in PROVENANCE_FIELDS}
    variants = []
    for part in parts:
        if any(part.get(field) != value for field, value in provenance.items()):
            raise ValueError("Schema variants have different source/version provenance")
        variants.extend(part["variants"])
    keys = [(variant["platform"], variant["architecture"]) for variant in variants]
    # A release must contain each required OS/architecture exactly once.
    if set(keys) != REQUIRED_VARIANTS or len(keys) != len(REQUIRED_VARIANTS):
        raise ValueError("Release needs exactly darwin/amd64, darwin/arm64 and windows/amd64")
    variants.sort(key=lambda variant: (variant["platform"], variant["architecture"]))
    return dict(provenance, variants=variants)


def write_artifact(artifact, output):
    # Publish JSON as a separate release file, not as content inside the binary.
    # Hash the exact bytes written to disk for the accompanying checksum file.
    raw = (json.dumps(artifact, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(raw)
    checksum = hashlib.sha256(raw).hexdigest() + "  " + output.name + "\n"
    output.with_name(output.name + ".sha256").write_text(checksum, encoding="ascii")


def verify_binary(artifact, binary):
    """Check version and SQL column/alias registration without reading table data."""
    # Run the newly built osquery executable with a temporary database/config.
    # Disable extensions and events so they do not change the inspected schema.
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

        # Ask the executable for its version, rather than trusting a file name.
        rows = query("select version from osquery_info;")
        if len(rows) != 1 or rows[0]["version"] != artifact["osqueryVersion"]:
            raise ValueError("Built binary version differs from schema version")
        for table in artifact["variants"][0]["tables"]:
            # Foreign and disabled tables are not supported native implementations.
            if table["availability"] != "native":
                continue
            all_columns = {column["name"] for column in table["columns"]}
            visible_columns = {column["name"] for column in table["columns"]
                               if not column["options"].get("hidden", False)}
            for name in [table["name"]] + table["aliases"]:
                # Canonical tables expose hidden columns; table aliases omit them.
                expected = all_columns if name == table["name"] else visible_columns
                escaped = name.replace("'", "''")
                columns = query("select name from pragma_table_xinfo('{}');".format(escaped))
                names = {column["name"] for column in columns}
                # Every advertised column must exist in the executable. This is
                # one-way: omitted tables/columns and column types are not checked.
                if not expected <= names:
                    raise ValueError("Built table {} missing columns {}".format(name, sorted(expected - names)))
            for column in table["columns"]:
                for alias in column["aliases"]:
                    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", alias):
                        raise ValueError("Unexpected SQL column alias: {}".format(alias))
                    quoted_table = table["name"].replace('"', '""')
                    # LIMIT 0 checks that SQL accepts the alias without fetching rows.
                    # This does not test JOINs, device permissions or actual table data.
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
        # Step 1: turn this build's selected .table files into an in-memory catalog.
        artifact = export_variant(args.source_root, args.native_specs, args.foreign_specs,
                                  args.platform, args.architecture, args.release_tag,
                                  args.osquery_version, args.source_commit)
        if args.binary:
            # Step 2: CI supplies --binary to verify the catalog against real osquery.
            # Any verification failure prevents this command from writing the JSON.
            verify_binary(artifact, args.binary)
    else:
        # Step 3: the release job merges the three already verified build catalogs.
        artifact = aggregate([json.loads(path.read_text(encoding="utf-8")) for path in args.variants])
    # Step 4: write the individual or combined catalog and its checksum file.
    write_artifact(artifact, args.output)
    print("Exported {} variant(s), {} tables: {}".format(
        len(artifact["variants"]), sum(len(v["tables"]) for v in artifact["variants"]), args.output))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        # Exit non-zero so CI stops instead of publishing an unverified release.
        print("Schema export failed: {}".format(error), file=sys.stderr)
        sys.exit(1)
