/* Reusable typed constants for built-in BattleTech Lua packages. */

#pragma once

#include <lua.h>
#include <stddef.h>

#include "mux/lua/packages/btech/btech_package.h"

/** One public constant name and its native integer value. */
typedef struct BtechLuaConstantEntry {
  const char *name;
  int value;
} BtechLuaConstantEntry;

/** Immutable namespace metadata shared by constant producers and consumers. */
typedef struct BtechLuaConstantCatalog {
  const char *qualified_name;
  const BtechLuaConstantEntry *entries;
  size_t entry_count;
} BtechLuaConstantCatalog;

/** Installs the metatables shared by all typed BattleTech constants. */
void lua_btech_constant_install_metatables(lua_State *state);

/**
 * Installs one immutable constant namespace on the table at the stack top.
 * @param[in,out] state Lua state whose top value is the parent namespace.
 * @param[in] package Package that owns the constants.
 * @param[in] field Field receiving the new namespace.
 * @param[in] catalog Constant definitions exposed by the namespace.
 */
void lua_btech_constant_install_namespace(
    lua_State *state, LuaBtechPackage *package, const char *field,
    const BtechLuaConstantCatalog *catalog);

/**
 * Requires a typed constant from a particular catalog and runtime.
 * @return Native value stored by the constant.
 */
int lua_btech_constant_require_at(LuaBtechPackage *package, lua_State *state,
                                  int index, int argument, const char *label,
                                  const BtechLuaConstantCatalog *catalog);

/** Pushes a typed constant selected by native value. */
void lua_btech_constant_push(lua_State *state, LuaBtechPackage *package,
                             const BtechLuaConstantCatalog *catalog, int value);
