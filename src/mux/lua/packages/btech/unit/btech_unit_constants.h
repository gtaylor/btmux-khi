/* Typed Lua constants shared by BattleTech unit-oriented packages. */

#pragma once

#include "btech/repair/mechrep_api.h"
#include "btech/unit/mech_api_types.h"
#include "btech/unit/section_types.h"
#include "mux/lua/packages/btech/btech_constants.h"

typedef struct BtechLuaTechnologySelection {
  BtechAdminTechnologyGroup group;
  int bit;
} BtechLuaTechnologySelection;

typedef enum BtechLuaTechnologyClearGroup : int {
  BTECH_LUA_TECHNOLOGY_GROUP_UNIT,
  BTECH_LUA_TECHNOLOGY_GROUP_INFANTRY,
  BTECH_LUA_TECHNOLOGY_GROUP_ALL,
} BtechLuaTechnologyClearGroup;

extern const BtechLuaConstantCatalog BTECH_LUA_UNIT_TYPES;
extern const BtechLuaConstantCatalog BTECH_LUA_MOVEMENT_TYPES;
extern const BtechLuaConstantCatalog BTECH_LUA_SECTIONS;
extern const BtechLuaConstantCatalog BTECH_LUA_TECHNOLOGY;
extern const BtechLuaConstantCatalog BTECH_LUA_TECHNOLOGY_GROUPS;
extern const BtechLuaConstantCatalog BTECH_LUA_FIRE_MODES;
extern const BtechLuaConstantCatalog BTECH_LUA_AMMUNITION_MODES;

/** Installs all immutable typed-constant namespaces on `btech.unit`. */
void lua_btech_unit_constants_install(lua_State *state,
                                      LuaBtechPackage *package);

/** Requires a section constant valid for the supplied unit. */
int lua_btech_section_require_at(LuaBtechPackage *package, lua_State *state,
                                 int index, int argument, const char *label,
                                 const Mech *mech);

/** Pushes the section constant corresponding to a native unit section. */
void lua_btech_section_push(lua_State *state, LuaBtechPackage *package,
                            const Mech *mech, int section);

/** Converts a technology constant value to its native group and bit. */
BtechLuaTechnologySelection lua_btech_technology_selection(lua_State *state,
                                                           int value);
