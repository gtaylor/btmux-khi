/* Reusable typed constants for built-in BattleTech Lua packages. */

#include "mux/lua/packages/btech/btech_constants.h"

#include <lauxlib.h>
#include <lua.h>
#include <string.h>

#include "mux/lua/lua_error.h"
#include "mux/lua/lua_error_codes.h"
#include "mux/lua/packages/btech/btech_package.h"
#include "mux/support/checked_storage.h"

static const char LUA_BTECH_CONSTANT_METATABLE[] = "btmux.btech_constant";
static const char LUA_BTECH_CONSTANT_NAMESPACE_METATABLE[] =
    "btmux.btech_constant_namespace";

typedef struct LuaBtechConstant {
  LuaBtechPackage *package;
  const BtechLuaConstantCatalog *catalog;
  const BtechLuaConstantEntry *entry;
} LuaBtechConstant;

typedef struct LuaBtechConstantNamespace {
  LuaBtechPackage *package;
  const BtechLuaConstantCatalog *catalog;
} LuaBtechConstantNamespace;

static const BtechLuaConstantEntry *
constant_find_name(const BtechLuaConstantCatalog *catalog, const char *name) {
  for (size_t index = 0; index < catalog->entry_count; index++) {
    const BtechLuaConstantEntry *entry =
        checked_storage_at_const(catalog->entries, catalog->entry_count,
                                 sizeof(*catalog->entries), index);
    if (strcmp(entry->name, name) == 0)
      return entry;
  }
  return nullptr;
}

static const BtechLuaConstantEntry *
constant_find_value(const BtechLuaConstantCatalog *catalog, int value) {
  for (size_t index = 0; index < catalog->entry_count; index++) {
    const BtechLuaConstantEntry *entry =
        checked_storage_at_const(catalog->entries, catalog->entry_count,
                                 sizeof(*catalog->entries), index);
    if (entry->value == value)
      return entry;
  }
  return nullptr;
}

static void constant_push_entry(lua_State *state, LuaBtechPackage *package,
                                const BtechLuaConstantCatalog *catalog,
                                const BtechLuaConstantEntry *entry) {
  LuaBtechConstant *constant = lua_newuserdata(state, sizeof(*constant));
  *constant = (LuaBtechConstant){
      .package = package, .catalog = catalog, .entry = entry};
  luaL_getmetatable(state, LUA_BTECH_CONSTANT_METATABLE);
  lua_setmetatable(state, -2);
}

/** @par LuaLS ignore btech __tostring -- Internal typed-constant metamethod. */
static int constant_tostring(lua_State *state) {
  const LuaBtechConstant *constant =
      luaL_checkudata(state, 1, LUA_BTECH_CONSTANT_METATABLE);
  lua_pushstring(state, constant->entry->name);
  return 1;
}

/** @par LuaLS ignore btech __eq -- Internal typed-constant metamethod. */
static int constant_equal(lua_State *state) {
  const LuaBtechConstant *left =
      luaL_checkudata(state, 1, LUA_BTECH_CONSTANT_METATABLE);
  const LuaBtechConstant *right =
      luaL_checkudata(state, 2, LUA_BTECH_CONSTANT_METATABLE);
  lua_pushboolean(state, left->package == right->package &&
                             left->catalog == right->catalog &&
                             left->entry->value == right->entry->value);
  return 1;
}

/** @par LuaLS ignore btech __newindex -- Internal immutability metamethod. */
static int constant_immutable(lua_State *state) {
  return lua_error_raise(state, LUA_ERROR_CODE_ARG_INVALID,
                         "BattleTech constants are immutable");
}

/** @par LuaLS ignore btech __index -- Internal constant-lookup metamethod. */
static int constant_namespace_index(lua_State *state) {
  const LuaBtechConstantNamespace *name_space =
      luaL_checkudata(state, 1, LUA_BTECH_CONSTANT_NAMESPACE_METATABLE);
  if (lua_type(state, 2) != LUA_TSTRING)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "%s constant name must be a string",
                         name_space->catalog->qualified_name);
  const char *name = lua_tostring(state, 2);
  const BtechLuaConstantEntry *entry =
      constant_find_name(name_space->catalog, name);
  if (!entry)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "unknown %s constant '%s'",
                         name_space->catalog->qualified_name, name);
  constant_push_entry(state, name_space->package, name_space->catalog, entry);
  return 1;
}

void lua_btech_constant_install_metatables(lua_State *state) {
  luaL_newmetatable(state, LUA_BTECH_CONSTANT_METATABLE);
  lua_pushcfunction(state, constant_tostring);
  lua_setfield(state, -2, "__tostring");
  lua_pushcfunction(state, constant_equal);
  lua_setfield(state, -2, "__eq");
  lua_pushcfunction(state, constant_immutable);
  lua_setfield(state, -2, "__newindex");
  lua_pushstring(state, "protected BattleTech constant metatable");
  lua_setfield(state, -2, "__metatable");
  lua_pop(state, 1);

  luaL_newmetatable(state, LUA_BTECH_CONSTANT_NAMESPACE_METATABLE);
  lua_pushcfunction(state, constant_namespace_index);
  lua_setfield(state, -2, "__index");
  lua_pushcfunction(state, constant_immutable);
  lua_setfield(state, -2, "__newindex");
  lua_pushstring(state, "protected BattleTech constant namespace metatable");
  lua_setfield(state, -2, "__metatable");
  lua_pop(state, 1);
}

void lua_btech_constant_install_namespace(
    lua_State *state, LuaBtechPackage *package, const char *field,
    const BtechLuaConstantCatalog *catalog) {
  LuaBtechConstantNamespace *name_space =
      lua_newuserdata(state, sizeof(*name_space));
  *name_space =
      (LuaBtechConstantNamespace){.package = package, .catalog = catalog};
  luaL_getmetatable(state, LUA_BTECH_CONSTANT_NAMESPACE_METATABLE);
  lua_setmetatable(state, -2);
  lua_setfield(state, -2, field);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Stack and public argument
// positions have different error-reporting roles.
int lua_btech_constant_require_at(LuaBtechPackage *package, lua_State *state,
                                  int index, int argument, const char *label,
                                  const BtechLuaConstantCatalog *catalog) {
  const LuaBtechConstant *constant =
      luaL_testudata(state, index, LUA_BTECH_CONSTANT_METATABLE);
  if (!constant || constant->package != package || constant->catalog != catalog)
    return lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                         "%s must be a %s constant from this runtime", label,
                         catalog->qualified_name);
  return constant->entry->value;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

void lua_btech_constant_push(lua_State *state, LuaBtechPackage *package,
                             const BtechLuaConstantCatalog *catalog,
                             int value) {
  const BtechLuaConstantEntry *entry = constant_find_value(catalog, value);
  if (!entry) {
    (void)lua_error_raise(state, LUA_ERROR_CODE_INTERNAL,
                          "unknown native value for %s",
                          catalog->qualified_name);
    return;
  }
  constant_push_entry(state, package, catalog, entry);
}
