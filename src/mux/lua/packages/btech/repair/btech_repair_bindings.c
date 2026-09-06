/* btech_repair_bindings.c - Native Lua bindings for btech.repair. */

#include <limits.h>
#include <lua.h>
#include <stddef.h>
#include <time.h>

#include "btech/configuration.h"
#include "btech/repair/mech_tech_api.h"
#include "btech/repair/mech_tech_commands_api.h"
#include "btech/repair/mech_tech_damages.h"
#include "btech/repair/mech_tech_damages_api.h"
#include "btech/repair/mechrep_api.h"
#include "btech/special/registry_api.h"
#include "btech/special_objects.h"
#include "btech/unit/mech_equipment_api.h"
#include "btech/unit/mech_utils_api.h"
#include "mux/lua/lua_error.h"
#include "mux/lua/lua_error_codes.h"
#include "mux/lua/packages/btech/btech_constants.h"
#include "mux/lua/packages/btech/btech_package.h"
#include "mux/lua/packages/btech/btech_package_internal.h"
#include "mux/lua/packages/btech/unit/btech_unit_constants.h"
#include "mux/objects/flags.h"
#include "mux/server/platform.h"
#include "mux/server/runtime_clock.h"

/**
 * @par LuaLS definition btech type btech.repair.immediate.records
 * @code{.lua}
 * ---@class (exact) BtechRepairOperationNamespace
 * ---@field REATTACH BtechRepairOperation
 * ---@field REPAIR_PART BtechRepairOperation
 * ---@field REPAIR_WEAPON_TEMPORARY BtechRepairOperation
 * ---@field REPAIR_ENHANCEMENT BtechRepairOperation
 * ---@field REPAIR_FOCUS BtechRepairOperation
 * ---@field REPAIR_CRYSTAL BtechRepairOperation
 * ---@field REPAIR_BARREL BtechRepairOperation
 * ---@field REPAIR_AMMO_FEED BtechRepairOperation
 * ---@field REPAIR_RANGING BtechRepairOperation
 * ---@field REPAIR_AMMO_MOUNT BtechRepairOperation
 * ---@field REPLACE_WEAPON BtechRepairOperation
 * ---@field RELOAD BtechRepairOperation
 * ---@field REPAIR_ARMOR BtechRepairOperation
 * ---@field REPAIR_REAR_ARMOR BtechRepairOperation
 * ---@field REPAIR_INTERNAL BtechRepairOperation
 * ---@field DETACH BtechRepairOperation
 * ---@field SCRAP_PART BtechRepairOperation
 * ---@field SCRAP_WEAPON BtechRepairOperation
 * ---@field UNLOAD BtechRepairOperation
 * ---@field RESEAL BtechRepairOperation
 * ---@field REPLACE_SUIT BtechRepairOperation
 * ---@class BtechRepairArmorRequest
 * ---@field operation BtechRepairOperation
 * ---@field section BtechSection
 * ---@field value integer
 * ---@class BtechRepairInternalRequest
 * ---@field operation BtechRepairOperation
 * ---@field section BtechSection
 * ---@field value integer
 * ---@class BtechRepairRearArmorRequest
 * ---@field operation BtechRepairOperation
 * ---@field section BtechSection
 * ---@field value integer
 * ---@class BtechRepairPartRequest
 * ---@field operation BtechRepairOperation
 * ---@field section BtechSection
 * ---@field slot integer
 * ---@class BtechRepairReattachRequest
 * ---@field operation BtechRepairOperation
 * ---@field section BtechSection
 * @endcode
 * @par LuaLS definition btech alias btech.repair.immediate
 * @code{.lua}
 * ---@alias BtechImmediateRepair BtechRepairArmorRequest|BtechRepairInternalRequest|BtechRepairRearArmorRequest|BtechRepairPartRequest|BtechRepairReattachRequest
 * @endcode
 * @par LuaLS definition btech callable btech.repair.apply
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param repair BtechImmediateRepair
 * function btech_repair.apply(unit, repair) end
 * @endcode
 */

static Mech *require_mech(lua_State *state, LuaBtechPackage *package) {
  const DbRef UNIT =
      lua_btech_require_special(package, state, 1, BTECH_SPECIAL_MECH, "unit");
  Mech *mech = btech_context_get_mech(lua_btech_context(package), UNIT);
  if (mech == nullptr)
    (void)lua_error_arg(state, 1, LUA_ERROR_CODE_OBJECT_UNAVAILABLE,
                        "unit runtime state is unavailable");
  return mech;
}

static const BtechLuaConstantEntry REPAIR_OPERATIONS[] = {
    {"REATTACH", REATTACH},
    {"REPAIR_PART", REPAIRP},
    {"REPAIR_WEAPON_TEMPORARY", REPAIRP_T},
    {"REPAIR_ENHANCEMENT", ENHCRIT_MISC},
    {"REPAIR_FOCUS", ENHCRIT_FOCUS},
    {"REPAIR_CRYSTAL", ENHCRIT_CRYSTAL},
    {"REPAIR_BARREL", ENHCRIT_BARREL},
    {"REPAIR_AMMO_FEED", ENHCRIT_AMMOB},
    {"REPAIR_RANGING", ENHCRIT_RANGING},
    {"REPAIR_AMMO_MOUNT", ENHCRIT_AMMOM},
    {"REPLACE_WEAPON", REPAIRG},
    {"RELOAD", RELOAD},
    {"REPAIR_ARMOR", FIXARMOR},
    {"REPAIR_REAR_ARMOR", FIXARMOR_R},
    {"REPAIR_INTERNAL", FIXINTERNAL},
    {"DETACH", DETACH},
    {"SCRAP_PART", SCRAPP},
    {"SCRAP_WEAPON", SCRAPG},
    {"UNLOAD", UNLOAD},
    {"RESEAL", RESEAL},
    {"REPLACE_SUIT", REPLACESUIT},
};

static const BtechLuaConstantCatalog REPAIR_OPERATION_CATALOG = {
    .qualified_name = "btech.repair.operations",
    .entries = REPAIR_OPERATIONS,
    .entry_count = sizeof(REPAIR_OPERATIONS) / sizeof(*REPAIR_OPERATIONS),
};
static_assert(sizeof(REPAIR_OPERATIONS) / sizeof(*REPAIR_OPERATIONS) ==
              NUM_DAMAGE_TYPES);

typedef struct RepairPushContext {
  lua_State *state;
  LuaBtechPackage *package;
  Mech *mech;
  int index;
} RepairPushContext;

static bool push_need(const BtechRepairNeed *need, void *opaque) {
  RepairPushContext *context = opaque;
  lua_State *state = context->state;
  lua_newtable(state);
  lua_btech_constant_push(state, context->package, &REPAIR_OPERATION_CATALOG,
                          need->operation);
  lua_setfield(state, -2, "operation");
  lua_btech_section_push(state, context->package, context->mech, need->section);
  lua_setfield(state, -2, "section");
  lua_pushboolean(state, (int)need->in_progress);
  lua_setfield(state, -2, "in_progress");
  if (need->operation == FIXARMOR || need->operation == FIXARMOR_R ||
      need->operation == FIXINTERNAL) {
    lua_pushinteger(state, need->detail);
    lua_setfield(state, -2, "amount");
  } else if (need->detail >= 0 && need->operation != REATTACH &&
             need->operation != DETACH && need->operation != RESEAL &&
             need->operation != REPLACESUIT) {
    lua_pushinteger(state, need->detail + 1);
    lua_setfield(state, -2, "slot");
  }
  lua_rawseti(state, -2, context->index++);
  return true;
}

static int lua_btech_repair_needs(lua_State *state, LuaBtechPackage *package) {
  Mech *mech = require_mech(state, package);
  lua_newtable(state);
  RepairPushContext context = {
      .state = state, .package = package, .mech = mech, .index = 1};
  btech_repair_needs_visit(mech, push_need, &context);
  return 1;
}

static int lua_btech_repair_is_under_repair(lua_State *state,
                                            LuaBtechPackage *package) {
  lua_pushboolean(state,
                  figure_latest_tech_event(require_mech(state, package)) > 0);
  return 1;
}

static int lua_btech_repair_is_fixable(lua_State *state,
                                       LuaBtechPackage *package) {
  lua_pushboolean(state, unit_is_fixable(require_mech(state, package)) ? 1 : 0);
  return 1;
}

static int lua_btech_repair_technician_available_in(lua_State *state,
                                                    LuaBtechPackage *package) {
  const DbRef PLAYER = lua_btech_require_object(package, state, 1);
  if (!is_player(package->services->database, PLAYER))
    return lua_error_arg(state, 1, LUA_ERROR_CODE_OBJECT_INVALID,
                         "object is not a player");
  const time_t AVAILABLE =
      btech_repair_technician_available_at(lua_btech_context(package), PLAYER);
  const time_t NOW = package->services->clock->now;
  lua_pushinteger(state, AVAILABLE > NOW ? (lua_Integer)(AVAILABLE - NOW) : 0);
  return 1;
}

static int constant_field(lua_State *state, LuaBtechPackage *package, int table,
                          const char *field,
                          const BtechLuaConstantCatalog *catalog) {
  lua_btech_get_field(state, table, field);
  const int VALUE =
      lua_btech_constant_require_at(package, state, -1, 2, field, catalog);
  lua_pop(state, 1);
  return VALUE;
}

static int repair_section(lua_State *state, LuaBtechPackage *package,
                          Mech *mech) {
  lua_btech_get_field(state, 2, "section");
  const int SECTION =
      lua_btech_section_require_at(package, state, -1, 2, "section", mech);
  lua_pop(state, 1);
  return SECTION;
}

static int lua_btech_repair_apply(lua_State *state, LuaBtechPackage *package) {
  static const char *const VALUE_FIELDS[] = {"operation", "section", "value"};
  static const char *const SLOT_FIELDS[] = {"operation", "section", "slot"};
  static const char *const SECTION_FIELDS[] = {"operation", "section"};
  Mech *mech = require_mech(state, package);
  if (lua_gettop(state) < 2)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "repair request is required");
  if (!lua_istable(state, 2))
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "repair request must be a table");
  const int OPERATION =
      constant_field(state, package, 2, "operation", &REPAIR_OPERATION_CATALOG);
  BtechAdminRepairKind kind;
  if (OPERATION == FIXARMOR) {
    lua_btech_check_options(state, 2, VALUE_FIELDS,
                            sizeof(VALUE_FIELDS) / sizeof(VALUE_FIELDS[0]), 2);
    kind = BTECH_ADMIN_REPAIR_ARMOR;
  } else if (OPERATION == FIXINTERNAL) {
    lua_btech_check_options(state, 2, VALUE_FIELDS,
                            sizeof(VALUE_FIELDS) / sizeof(VALUE_FIELDS[0]), 2);
    kind = BTECH_ADMIN_REPAIR_INTERNAL;
  } else if (OPERATION == FIXARMOR_R) {
    lua_btech_check_options(state, 2, VALUE_FIELDS,
                            sizeof(VALUE_FIELDS) / sizeof(VALUE_FIELDS[0]), 2);
    kind = BTECH_ADMIN_REPAIR_REAR_ARMOR;
  } else if (OPERATION == REPAIRP) {
    lua_btech_check_options(state, 2, SLOT_FIELDS,
                            sizeof(SLOT_FIELDS) / sizeof(SLOT_FIELDS[0]), 2);
    kind = BTECH_ADMIN_REPAIR_CRITICAL;
  } else if (OPERATION == REATTACH) {
    lua_btech_check_options(state, 2, SECTION_FIELDS,
                            sizeof(SECTION_FIELDS) / sizeof(SECTION_FIELDS[0]),
                            2);
    kind = BTECH_ADMIN_REPAIR_REATTACH;
  } else {
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "operation is not an immediate repair operation");
  }
  const int SECTION = repair_section(state, package, mech);
  if (kind == BTECH_ADMIN_REPAIR_REAR_ARMOR &&
      !btech_admin_section_has_rear_armor(mech, SECTION))
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "rear armor is only valid on Mech torso sections");
  int value = 0;
  if (kind == BTECH_ADMIN_REPAIR_ARMOR || kind == BTECH_ADMIN_REPAIR_INTERNAL ||
      kind == BTECH_ADMIN_REPAIR_REAR_ARMOR) {
    value =
        (int)lua_btech_check_integer_field(state, 2, "value", 0, UCHAR_MAX, 2);
  } else if (kind == BTECH_ADMIN_REPAIR_CRITICAL) {
    value = (int)lua_btech_check_integer_field(
                state, 2, "slot", 1, mech_section_critical_count(mech, SECTION),
                2) -
            1;
  }
  btech_admin_repair(mech, kind, SECTION, value);
  return 0;
}

static const BtechLuaNativeEntry BTECH_REPAIR_ENTRIES[] = {
    {"apply", "repair.apply", lua_btech_repair_apply},
    {"needs", "repair.needs", lua_btech_repair_needs},
    {"is_under_repair", "repair.is_under_repair",
     lua_btech_repair_is_under_repair},
    {"is_fixable", "repair.is_fixable", lua_btech_repair_is_fixable},
    {"technician_available_in", "repair.technician_available_in",
     lua_btech_repair_technician_available_in},
};

void lua_btech_install_repair_bindings(lua_State *state,
                                       LuaBtechPackage *package) {
  lua_btech_install_native_bindings(
      state, package, "repair", BTECH_REPAIR_ENTRIES,
      sizeof(BTECH_REPAIR_ENTRIES) / sizeof(BTECH_REPAIR_ENTRIES[0]));
  lua_getfield(state, -1, "repair");
  lua_btech_constant_install_namespace(state, package, "operations",
                                       &REPAIR_OPERATION_CATALOG);
  lua_pop(state, 1);
}
