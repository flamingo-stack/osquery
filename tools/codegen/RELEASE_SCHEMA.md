# Released osquery schema

The osquery release publishes `osquery-schema.json` and
`osquery-schema.json.sha256` alongside the macOS universal and Windows binaries.
The JSON is generated from the same checkout and CMake-selected specs as each
binary. Publication requires all three build variants and matching provenance.

## Contract (formatVersion 1)

- `releaseTag`: exact GitHub release tag, including a leading `v` when supplied.
- `osqueryVersion`: resolved CMake engine version (the fork release version).
- `sourceCommit`: full 40-character checkout commit SHA.
- `variants`: Darwin amd64, Darwin arm64, Windows amd64; each has `platform`,
  `architecture` and `tables`.
- Table: `name`, `aliases` (string array), `description`, `notes`, `url` (strings),
  `examples` (string array), `availability`, `attributes` (spec attribute object),
  `foreignKeys` (`{column, table}` array), `columns`.
- Column: `name`, `aliases` (string array), SQL `type` (lowercase), `description`,
  `notes`, `platforms` (string array), `options` (spec options object such as
  `required`, `hidden`, `index`, `optimized`, `additional`, `collate`).

`availability` is `native`, `foreign` or `disabled`. Foreign tables are registered
empty-result placeholders. They do not establish native platform support.
Platform-specific columns retain their platform list; an empty list means no
column-level restriction within that variant. Hidden columns can be selected
explicitly and are not automatically invalid SQL. Runtime event configuration,
permissions and extensions remain separate from compiled schema availability.

The checksum is SHA-256 of the exact complete JSON asset bytes, before decoding
or reserialization. The sidecar uses `HASH  osquery-schema.json\n`, with a lowercase
64-character hexadecimal hash. There is no embedded checksum or canonical JSON
requirement. `SHA256SUMS` additionally covers the schema and binary archives.

## Build and release checks

CMake writes `build/specs/native_specs.txt` and `foreign_specs.txt` using its actual
selected source lists. `release_schema.py variant` reads those manifests, applies
the target platform to the upstream spec parser, and verifies version, table,
column and alias registration using the just-built executable in shell mode.
Metadata queries do not execute the table's data collection query.

`release_schema.py aggregate` requires exactly the three supported variants with
the same format, release tag, engine version and source commit. Release and PR CI
run the exporter tests; each platform build verifies its own schema.

The existing mutable `latest` prerelease retains its exact source and engine
provenance. It does not prove a device has that build. Consumers must distinguish
catalog provenance from verified installed platform, architecture and version.
Older releases without an artifact remain unconfirmed rather than falling back
to another version's schema.

Run the local exporter tests with:

```sh
python3 -B -m unittest discover -s tools/codegen -p test_release_schema.py -v
```
