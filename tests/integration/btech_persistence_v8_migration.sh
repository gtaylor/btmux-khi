#!/usr/bin/env bash
set -euo pipefail

if (( $# != 1 )); then
  echo "usage: $0 MIGRATION_SQL" >&2
  exit 2
fi

if ! command -v sqlite3 >/dev/null 2>&1; then
  echo "skipping: sqlite3 CLI is unavailable" >&2
  exit 77
fi

migration=$1
artifact_root=${TMPDIR:-/tmp}
test_directory=$(mktemp -d "${artifact_root%/}/btmux-v8-migration.XXXXXX")

finish() {
  rm -rf -- "$test_directory"
}
trap finish EXIT

for schema_version in 6 7; do
  database=$test_directory/version-$schema_version.db
  sqlite3 "$database" <<SQL
CREATE TABLE btech_persistence_metadata (
  id INTEGER PRIMARY KEY,
  schema_name TEXT NOT NULL,
  schema_version INTEGER NOT NULL
);
INSERT INTO btech_persistence_metadata
VALUES (1, 'stompymux-btech', $schema_version);

CREATE TABLE btech_autopilots (
  dbref INTEGER PRIMARY KEY,
  flags INTEGER NOT NULL,
  mech_dbref INTEGER NOT NULL
);
INSERT INTO btech_autopilots VALUES
  (10, 1, -1),
  (11, 0, 21),
  (12, 0, 22),
  (13, 1, -1),
  (14, 0, 24);

CREATE TABLE objects (dbref INTEGER PRIMARY KEY, location INTEGER NOT NULL);
INSERT INTO objects VALUES (10, -1), (11, 21), (12, 22);

CREATE TABLE btech_mech_runtime (
  mech_dbref INTEGER PRIMARY KEY,
  autopilot_num INTEGER NOT NULL
);
INSERT INTO btech_mech_runtime VALUES (21, 11), (22, 12), (24, 14);

CREATE TABLE btech_autopilot_commands (
  autopilot_dbref INTEGER NOT NULL,
  position INTEGER NOT NULL
);
INSERT INTO btech_autopilot_commands VALUES (11, 0), (14, 0);
SQL

  sqlite3 "$database" < "$migration"

  result=$(sqlite3 -separator '|' "$database" <<'SQL'
SELECT schema_version FROM btech_persistence_metadata WHERE id = 1;
SELECT group_concat(dbref || ':' || engaged, ',')
FROM (SELECT dbref, engaged FROM btech_autopilots ORDER BY dbref);
SELECT [notnull] || ':' || dflt_value
FROM pragma_table_info('btech_autopilots') WHERE name = 'engaged';
SQL
  )

  expected=$'8\n10:1,11:1,12:0,13:1,14:0\n1:0'
  if [[ $result != "$expected" ]]; then
    echo "unexpected v$schema_version-to-v8 migration result:" >&2
    printf '%s\n' "$result" >&2
    exit 1
  fi

  if sqlite3 "$database" < "$migration" >/dev/null 2>&1; then
    echo "v8 migration unexpectedly accepted an already-migrated database" >&2
    exit 1
  fi
  if [[ $(sqlite3 "$database" \
    "SELECT schema_version || ':' || count(*) FROM btech_persistence_metadata, btech_autopilots WHERE id=1;") != "8:5" ]]; then
    echo "failed migration guard changed the current database" >&2
    exit 1
  fi
done
