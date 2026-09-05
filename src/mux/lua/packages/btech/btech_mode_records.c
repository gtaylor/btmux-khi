/* btech_mode_records.c - Structured critical-slot mode records for Lua. */

#include <lua.h>
#include <stdbool.h>
#include <stddef.h>

#include "mux/lua/packages/btech/btech_constants.h"
#include "mux/lua/packages/btech/btech_package.h"
#include "mux/lua/packages/btech/btech_package_internal.h"
#include "mux/lua/packages/btech/unit/btech_unit_constants.h"
#include "mux/support/checked_storage.h"

void lua_btech_push_critical_modes(lua_State *state, LuaBtechPackage *package,
                                   unsigned int modes, bool ammunition) {
  const BtechLuaConstantCatalog *catalog =
      ammunition ? &BTECH_LUA_AMMUNITION_MODES : &BTECH_LUA_FIRE_MODES;
  int output = 1;
  lua_newtable(state);
  for (size_t index = 0; index < catalog->entry_count; index++) {
    const BtechLuaConstantEntry *entry =
        checked_storage_at_const(catalog->entries, catalog->entry_count,
                                 sizeof(*catalog->entries), index);
    if ((modes & (unsigned int)entry->value) == 0)
      continue;
    lua_btech_constant_push(state, package, catalog, entry->value);
    lua_rawseti(state, -2, output++);
  }
}
