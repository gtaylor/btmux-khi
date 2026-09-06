#include <sqlite3.h>

#undef NDEBUG
#include <assert.h>

#include "autopilot.h"
#include "btech/persistence/sqlite_internal.h"
#include "context_internal.h" // IWYU pragma: keep
#include "mux/objects/db.h"
#include "mux/objects/flags.h"
#include "mux/server/platform.h"
#include "registry_api.h"
#include "special_object.h"

enum : DbRef {
  FLAGS_ORPHAN = 10,
  CONTAINMENT_ORPHAN = 11,
  CONTAINED_AUTOPILOT = 12,
};

static Autopilot flags_orphan;
static Autopilot containment_orphan;
static Autopilot contained_autopilot;

bool is_good_obj(GameDatabase *database [[maybe_unused]], DbRef x) {
  return x == FLAGS_ORPHAN || x == CONTAINMENT_ORPHAN ||
         x == CONTAINED_AUTOPILOT;
}

int btech_context_which_special(BtechContext *context [[maybe_unused]],
                                DbRef key) {
  return is_good_obj(nullptr, key) ? GTYPE_AUTO : -1;
}

void *btech_context_find_object(BtechContext *context [[maybe_unused]],
                                DbRef key) {
  switch (key) {
  case FLAGS_ORPHAN:
    return &flags_orphan;
  case CONTAINMENT_ORPHAN:
    return &containment_orphan;
  case CONTAINED_AUTOPILOT:
    return &contained_autopilot;
  default:
    return nullptr;
  }
}

int btech_special_prepare_v2(sqlite3 *sqlite, const char *sql, int byte_count,
                             sqlite3_stmt **statement, const char **tail) {
  return sqlite3_prepare_v2(sqlite, sql, byte_count, statement, tail);
}

int btech_special_column_int(sqlite3_stmt *statement, int column, int *value) {
  if (sqlite3_column_type(statement, column) != SQLITE_INTEGER)
    return -1;
  *value = sqlite3_column_int(statement, column);
  return 0;
}

int btech_special_column_long(sqlite3_stmt *statement, int column,
                              long *value) {
  if (sqlite3_column_type(statement, column) != SQLITE_INTEGER)
    return -1;
  *value = (long)sqlite3_column_int64(statement, column);
  return 0;
}

int btech_special_schema_version(sqlite3 *sqlite, int *version) {
  sqlite3_stmt *statement = nullptr;
  if (sqlite3_prepare_v2(
          sqlite,
          "SELECT schema_version FROM btech_persistence_metadata WHERE id=1;",
          -1, &statement, nullptr) != SQLITE_OK)
    return -1;
  int result = -1;
  if (sqlite3_step(statement) == SQLITE_ROW) {
    *version = sqlite3_column_int(statement, 0);
    result = 0;
  }
  sqlite3_finalize(statement);
  return result;
}

static void execute(sqlite3 *sqlite, const char *sql) {
  assert(sqlite3_exec(sqlite, sql, nullptr, nullptr, nullptr) == SQLITE_OK);
}

static void check_legacy_version(sqlite3 *sqlite, BtechContext *context,
                                 int version) {
  sqlite3_stmt *statement = nullptr;
  assert(sqlite3_prepare_v2(
             sqlite, "UPDATE btech_persistence_metadata SET schema_version=?;",
             -1, &statement, nullptr) == SQLITE_OK);
  assert(sqlite3_bind_int(statement, 1, version) == SQLITE_OK);
  assert(sqlite3_step(statement) == SQLITE_DONE);
  sqlite3_finalize(statement);

  flags_orphan.engaged = false;
  containment_orphan.engaged = true;
  contained_autopilot.engaged = false;
  assert(btech_special_finalize_autopilot_engagement(sqlite, context) == 0);
  assert(flags_orphan.engaged);
  assert(!containment_orphan.engaged);
  assert(contained_autopilot.engaged);
}

int main(void) {
  sqlite3 *sqlite = nullptr;
  GameDatabase database = {.object_storage = nullptr};
  BtechContext context = {.database = &database};

  assert(sqlite3_open(":memory:", &sqlite) == SQLITE_OK);
  execute(sqlite,
          "CREATE TABLE btech_persistence_metadata (id INTEGER PRIMARY KEY, "
          "schema_name TEXT NOT NULL, schema_version INTEGER NOT NULL);"
          "INSERT INTO btech_persistence_metadata VALUES "
          "(1, 'stompymux-btech', 6);"
          "CREATE TABLE btech_autopilots "
          "(dbref INTEGER PRIMARY KEY, flags INTEGER NOT NULL, "
          "mech_dbref INTEGER NOT NULL);"
          "INSERT INTO btech_autopilots VALUES "
          "(10, 1, -1), (11, 0, 21), (12, 0, 22);"
          "CREATE TABLE objects (dbref INTEGER PRIMARY KEY, "
          "location INTEGER NOT NULL);"
          "INSERT INTO objects VALUES (12, 22);"
          "CREATE TABLE btech_mech_runtime "
          "(mech_dbref INTEGER PRIMARY KEY, autopilot_num INTEGER NOT NULL);"
          "INSERT INTO btech_mech_runtime VALUES (21, 11), (22, 12);"
          "CREATE TABLE btech_autopilot_commands "
          "(autopilot_dbref INTEGER NOT NULL, position INTEGER NOT NULL);"
          "INSERT INTO btech_autopilot_commands VALUES (11, 0), (12, 0);");

  check_legacy_version(sqlite, &context, 6);
  check_legacy_version(sqlite, &context, 7);
  sqlite3_close(sqlite);
  return 0;
}
