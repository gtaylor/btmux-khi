/* Typed Lua administration operations for live BattleTech units. */

#include <limits.h>
#include <lua.h>
#include <math.h>
#include <stddef.h>
#include <strings.h>

#include "btech/repair/mechrep_api.h"
#include "btech/special/registry_api.h"
#include "btech/special_objects.h"
#include "btech/unit/equipment_types.h"
#include "btech/unit/mech_build_api.h"
#include "btech/unit/mech_classification_api.h"
#include "btech/unit/mech_equipment_api.h"
#include "btech/unit/mech_partnames_api.h"
#include "btech/unit/mech_specification_api.h"
#include "btech/unit/mech_utils_api.h"
#include "btech/unit/section_types.h"
#include "btech/unit/template_api.h"
#include "btech/unit/weapon_catalogue_api.h"
#include "mux/lua/lua_error.h"
#include "mux/lua/lua_error_codes.h"
#include "mux/lua/packages/btech/btech_package.h"
#include "mux/lua/packages/btech/btech_package_internal.h"
#include "mux/objects/flags.h"
#include "mux/server/platform.h"
#include "mux/support/checked_storage.h"

/**
 * @par LuaLS definition btech type btech.unit.admin
 * @code{.lua}
 * ---@class BtechWeaponInstall
 * ---@field part BtechPartRef
 * ---@field section string
 * ---@field slots integer[]
 * ---@field rear_facing? boolean
 * ---@field targeting_computer? boolean
 * ---@field one_shot? boolean
 * ---@class BtechAmmunitionConfiguration
 * ---@field weapon BtechPartRef
 * ---@field section string
 * ---@field slot integer
 * ---@field half_ton? boolean
 * ---@field ammunition_modes? string[]
 * ---@class BtechWeaponModes
 * ---@field fire_modes? string[]
 * ---@field ammunition_modes? string[]
 * ---@class BtechSpecialInstall
 * ---@field part? BtechPartRef Omit to empty the slot.
 * ---@field section string
 * ---@field slot integer
 * ---@field auxiliary_data? integer
 * @endcode
 *
 * @par LuaLS definition btech callable btech.unit.restore
 * @code{.lua}
 * ---@param unit DbRef|Object
 * function btech_unit.restore(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.save_template
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param reference string
 * function btech_unit.save_template(unit, reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.reset_critical_slots
 * @code{.lua}
 * ---@param unit DbRef|Object
 * function btech_unit.reset_critical_slots(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.install_weapon
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param request BtechWeaponInstall
 * function btech_unit.install_weapon(unit, request) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.configure_ammunition
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param request BtechAmmunitionConfiguration
 * function btech_unit.configure_ammunition(unit, request) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.restock_ammunition
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param section string
 * ---@param slot integer
 * function btech_unit.restock_ammunition(unit, section, slot) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_weapon_modes
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param weapon_number integer
 * ---@param modes BtechWeaponModes
 * function btech_unit.set_weapon_modes(unit, weapon_number, modes) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.install_special
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param request BtechSpecialInstall
 * function btech_unit.install_special(unit, request) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.add_technology
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param technology string
 * function btech_unit.add_technology(unit, technology) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.remove_technology
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param technology string
 * function btech_unit.remove_technology(unit, technology) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.clear_technologies
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param group "unit"|"infantry"|"all"
 * function btech_unit.clear_technologies(unit, group) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_unit_type
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param unit_type string
 * function btech_unit.set_unit_type(unit, unit_type) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_movement_type
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param movement_type string
 * function btech_unit.set_movement_type(unit, movement_type) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_jump_speed
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param movement_points number
 * function btech_unit.set_jump_speed(unit, movement_points) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_heat_sinks
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param count integer
 * function btech_unit.set_heat_sinks(unit, count) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_long_range_sensor_range
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param range integer
 * function btech_unit.set_long_range_sensor_range(unit, range) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_tactical_range
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param range integer
 * function btech_unit.set_tactical_range(unit, range) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_scan_range
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param range integer
 * function btech_unit.set_scan_range(unit, range) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_radio_quality
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param quality integer
 * function btech_unit.set_radio_quality(unit, quality) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_radio_range
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param range integer
 * function btech_unit.set_radio_range(unit, range) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_cargo_capacity
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param space integer
 * ---@param maximum_tons integer
 * function btech_unit.set_cargo_capacity(unit, space, maximum_tons) end
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

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Minimum and maximum form a
// named range.
static int require_integer(lua_State *state, int argument, int minimum,
                           int maximum, const char *label) {
  if (lua_type(state, argument) != LUA_TNUMBER)
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s must be an integer", label);
  const lua_Number NUMBER = lua_tonumber(state, argument);
  if (!isfinite(NUMBER) || floor(NUMBER) != NUMBER || NUMBER < minimum ||
      NUMBER > maximum)
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s is outside its valid range", label);
  return (int)NUMBER;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Minimum and maximum form a
// named range.
static lua_Number require_number(lua_State *state, int argument,
                                 lua_Number minimum, lua_Number maximum,
                                 const char *label) {
  if (lua_type(state, argument) != LUA_TNUMBER)
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s must be a number", label);
  const lua_Number NUMBER = lua_tonumber(state, argument);
  if (!isfinite(NUMBER) || NUMBER < minimum || NUMBER > maximum)
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s is outside its valid range", label);
  return NUMBER;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

static int section_field(lua_State *state, Mech *mech, int table,
                         int argument) {
  const char *wanted =
      lua_btech_check_string_field(state, table, "section", 64, argument);
  const UnitSectionCatalog CATALOG = {
      .unit_type = mech_class(mech), .movement_type = mech_movement_type(mech)};
  for (size_t index = 0; index < unit_section_name_count(&CATALOG); index++) {
    const char *name = unit_section_name(&CATALOG, index);
    const ArmorSectionAbbreviation ABBREVIATION = armor_section_abbreviation(
        &(ArmorSectionReference){.unit_class = mech_class(mech),
                                 .movement_type = mech_movement_type(mech),
                                 .location = (int)index});
    if (strcasecmp(wanted, name) == 0 ||
        strcasecmp(wanted, ABBREVIATION.text) == 0)
      return (int)index;
  }
  (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                      "unknown section");
  return -1;
}

static bool optional_boolean_field(lua_State *state, int table,
                                   const char *field, int argument) {
  lua_btech_get_field(state, table, field);
  if (lua_isnil(state, -1)) {
    lua_pop(state, 1);
    return false;
  }
  if (!lua_isboolean(state, -1))
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s must be a boolean", field);
  const bool VALUE = lua_toboolean(state, -1) != 0;
  lua_pop(state, 1);
  return VALUE;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Fallback and argument have
// distinct documented roles.
static int optional_integer_field(lua_State *state, int table,
                                  const char *field, int fallback,
                                  int argument) {
  lua_btech_get_field(state, table, field);
  if (lua_isnil(state, -1)) {
    lua_pop(state, 1);
    return fallback;
  }
  if (lua_type(state, -1) != LUA_TNUMBER)
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s must be an integer", field);
  const lua_Number NUMBER = lua_tonumber(state, -1);
  lua_pop(state, 1);
  if (!isfinite(NUMBER) || floor(NUMBER) != NUMBER || NUMBER < INT_MIN ||
      NUMBER > INT_MAX)
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s is outside its valid range", field);
  return (int)NUMBER;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

static PartReference part_field(lua_State *state, LuaBtechPackage *package,
                                int table, const char *field, int argument) {
  lua_btech_get_field(state, table, field);
  PartReference part = {0};
  if (!lua_btech_check_part(state, lua_btech_context(package), -1, argument,
                            &part))
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_BTECH_PART_NOT_FOUND,
                        "%s was not found", field);
  lua_pop(state, 1);
  return part;
}

typedef const char *CatalogName(size_t index);

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Count and argument have
// distinct documented roles.
static int named_bits(lua_State *state, int table, const char *field,
                      CatalogName *name_at, size_t count, int argument) {
  lua_btech_get_field(state, table, field);
  if (lua_isnil(state, -1)) {
    lua_pop(state, 1);
    return 0;
  }
  if (!lua_istable(state, -1))
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s must be an array", field);
  const int ARRAY = lua_gettop(state);
  const size_t LENGTH = lua_objlen(state, ARRAY);
  unsigned int bits = 0;
  for (size_t item = 1; item <= LENGTH; item++) {
    lua_rawgeti(state, ARRAY, (int)item);
    if (lua_type(state, -1) != LUA_TSTRING)
      (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                          "%s entries must be strings", field);
    const char *wanted = lua_tostring(state, -1);
    bool found = false;
    for (size_t index = 0; index < count; index++) {
      if (strcasecmp(wanted, name_at(index)) == 0) {
        bits |= 1U << index;
        found = true;
        break;
      }
    }
    lua_pop(state, 1);
    if (!found)
      (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                          "unknown %s value '%s'", field, wanted);
  }
  lua_pop(state, 1);
  return (int)bits;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

static int lua_btech_unit_restore(lua_State *state, LuaBtechPackage *package) {
  if (!btech_admin_template_restore(GOD, require_mech(state, package)))
    return lua_btech_operation_error(state, "template_restore_failed",
                                     "unable to restore unit template");
  return 0;
}

static int lua_btech_unit_save_template(lua_State *state,
                                        LuaBtechPackage *package) {
  Mech *mech = require_mech(state, package);
  if (lua_type(state, 2) != LUA_TSTRING || lua_objlen(state, 2) == 0 ||
      lua_objlen(state, 2) > 255)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "reference must contain 1 to 255 bytes");
  const char *reference = lua_tostring(state, 2);
  lua_btech_validate_resource_name(state, 2, reference, "reference");
  if (!btech_admin_template_save(GOD, mech, reference))
    return lua_btech_operation_error(state, "template_save_failed",
                                     "unable to save unit template");
  return 0;
}

static int lua_btech_unit_reset_critical_slots(lua_State *state,
                                               LuaBtechPackage *package) {
  btech_admin_criticals_reset(require_mech(state, package));
  return 0;
}

static int lua_btech_unit_install_weapon(lua_State *state,
                                         LuaBtechPackage *package) {
  static const char *const FIELDS[] = {
      "part",    "section", "slots", "rear_facing", "targeting_computer",
      "one_shot"};
  Mech *mech = require_mech(state, package);
  lua_btech_check_options(state, 2, FIELDS, sizeof(FIELDS) / sizeof(FIELDS[0]),
                          2);
  const PartReference PART = part_field(state, package, 2, "part", 2);
  if (!equipment_is_weapon(PART.id))
    return lua_error_arg(state, 2, LUA_ERROR_CODE_BTECH_PART_WRONG_KIND,
                         "part must be a weapon");
  const int SECTION = section_field(state, mech, 2, 2);
  lua_btech_get_field(state, 2, "slots");
  if (!lua_istable(state, -1))
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "slots must be an array");
  const size_t COUNT = lua_objlen(state, -1);
  const int WEAPON = weapon_from_equipment_index(PART.id);
  const int REQUIRED = get_weapon_crits(mech, WEAPON);
  if (COUNT == 0 || COUNT > NUM_CRITICALS ||
      (COUNT != (size_t)REQUIRED && REQUIRED < 9) || COUNT > (size_t)REQUIRED)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "slots must contain the required critical slots");
  int slots[NUM_CRITICALS];
  for (size_t index = 0; index < COUNT; index++) {
    lua_rawgeti(state, -1, (int)index + 1);
    const int SLOT =
        require_integer(state, -1, 1,
                        mech_section_critical_count(mech, SECTION), "slot") -
        1;
    lua_pop(state, 1);
    for (size_t previous = 0; previous < index; previous++)
      if (*(const int *)checked_storage_at_const(
              slots, NUM_CRITICALS, sizeof(*slots), previous) == SLOT)
        return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                             "slots must be unique");
    *(int *)checked_storage_at(slots, NUM_CRITICALS, sizeof(*slots), index) =
        SLOT;
  }
  lua_pop(state, 1);
  int modes = 0;
  if (optional_boolean_field(state, 2, "rear_facing", 2))
    modes |= REAR_MOUNT;
  if (optional_boolean_field(state, 2, "targeting_computer", 2))
    modes |= ON_TC;
  if (optional_boolean_field(state, 2, "one_shot", 2))
    modes |= OS_MODE;
  btech_admin_weapon_install(mech, WEAPON, slots, COUNT, SECTION, modes);
  return 0;
}

static int lua_btech_unit_configure_ammunition(lua_State *state,
                                               LuaBtechPackage *package) {
  static const char *const FIELDS[] = {"weapon", "section", "slot", "half_ton",
                                       "ammunition_modes"};
  Mech *mech = require_mech(state, package);
  lua_btech_check_options(state, 2, FIELDS, sizeof(FIELDS) / sizeof(FIELDS[0]),
                          2);
  const PartReference PART = part_field(state, package, 2, "weapon", 2);
  if (!equipment_is_weapon(PART.id))
    return lua_error_arg(state, 2, LUA_ERROR_CODE_BTECH_PART_WRONG_KIND,
                         "weapon must identify a weapon");
  const int WEAPON = weapon_from_equipment_index(PART.id);
  if (weapon_catalogue_ammunition_per_ton(WEAPON) == 0)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "weapon does not use ammunition");
  const int SECTION = section_field(state, mech, 2, 2);
  const int SLOT =
      (int)lua_btech_check_integer_field(
          state, 2, "slot", 1, mech_section_critical_count(mech, SECTION), 2) -
      1;
  const int FIRE_MODE_BITS =
      optional_boolean_field(state, 2, "half_ton", 2) ? HALFTON_MODE : 0;
  const int AMMUNITION_MODE_BITS =
      named_bits(state, 2, "ammunition_modes", template_critical_ammo_mode_name,
                 template_critical_ammo_mode_count(), 2);
  btech_admin_ammunition_configure(mech, WEAPON, SECTION, SLOT, FIRE_MODE_BITS,
                                   AMMUNITION_MODE_BITS);
  return 0;
}

static int lua_btech_unit_restock_ammunition(lua_State *state,
                                             LuaBtechPackage *package) {
  Mech *mech = require_mech(state, package);
  if (lua_type(state, 2) != LUA_TSTRING)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "section must be a string");
  lua_newtable(state);
  lua_pushvalue(state, 2);
  lua_setfield(state, -2, "section");
  const int SECTION = section_field(state, mech, -1, 2);
  lua_pop(state, 1);
  const int SLOT =
      require_integer(state, 3, 1, mech_section_critical_count(mech, SECTION),
                      "slot") -
      1;
  const int PART = mech_critical_part_type(mech, SECTION, SLOT);
  if (!equipment_is_ammunition(PART))
    return lua_error_arg(state, 3, LUA_ERROR_CODE_BTECH_PART_WRONG_KIND,
                         "critical slot is not ammunition");
  if (mech_critical_is_destroyed(mech, SECTION, SLOT))
    return lua_btech_operation_error(
        state, "ammunition_destroyed",
        "destroyed ammunition cannot be restocked");
  btech_admin_ammunition_restock(mech, SECTION, SLOT);
  return 0;
}

static int lua_btech_unit_set_weapon_modes(lua_State *state,
                                           LuaBtechPackage *package) {
  static const char *const FIELDS[] = {"fire_modes", "ammunition_modes"};
  Mech *mech = require_mech(state, package);
  const int NUMBER = require_integer(state, 2, 0, INT_MAX, "weapon_number");
  lua_btech_check_options(state, 3, FIELDS, sizeof(FIELDS) / sizeof(FIELDS[0]),
                          3);
  const int FIRE_MODE_BITS =
      named_bits(state, 3, "fire_modes", template_critical_fire_mode_name,
                 template_critical_fire_mode_count(), 3);
  const int AMMUNITION_MODE_BITS =
      named_bits(state, 3, "ammunition_modes", template_critical_ammo_mode_name,
                 template_critical_ammo_mode_count(), 3);
  if (!btech_admin_weapon_modes_set(mech, NUMBER, FIRE_MODE_BITS,
                                    AMMUNITION_MODE_BITS))
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "weapon number is not mounted");
  return 0;
}

static int lua_btech_unit_install_special(lua_State *state,
                                          LuaBtechPackage *package) {
  static const char *const FIELDS[] = {"part", "section", "slot",
                                       "auxiliary_data"};
  Mech *mech = require_mech(state, package);
  lua_btech_check_options(state, 2, FIELDS, sizeof(FIELDS) / sizeof(FIELDS[0]),
                          2);
  int special = -1;
  lua_btech_get_field(state, 2, "part");
  if (!lua_isnil(state, -1)) {
    PartReference part = {0};
    if (!lua_btech_check_part(state, lua_btech_context(package), -1, 2, &part))
      return lua_error_arg(state, 2, LUA_ERROR_CODE_BTECH_PART_NOT_FOUND,
                           "part was not found");
    if (!equipment_is_special(part.id))
      return lua_error_arg(state, 2, LUA_ERROR_CODE_BTECH_PART_WRONG_KIND,
                           "part must be special equipment");
    special = special_from_equipment_index(part.id);
  }
  lua_pop(state, 1);
  const int SECTION = section_field(state, mech, 2, 2);
  const int SLOT =
      (int)lua_btech_check_integer_field(
          state, 2, "slot", 1, mech_section_critical_count(mech, SECTION), 2) -
      1;
  const int DATA = optional_integer_field(state, 2, "auxiliary_data", 0, 2);
  btech_admin_special_install(mech, special, SECTION, SLOT, DATA);
  return 0;
}

static bool technology_find(const char *wanted,
                            BtechAdminTechnologyGroup *group, int *bit) {
  const struct {
    BtechAdminTechnologyGroup group;
    size_t count;
    CatalogName *name_at;
  } CATALOGS[] = {
      {BTECH_ADMIN_TECHNOLOGY_PRIMARY, primary_technology_name_count(),
       primary_technology_name},
      {BTECH_ADMIN_TECHNOLOGY_SECONDARY, secondary_technology_name_count(),
       secondary_technology_name},
      {BTECH_ADMIN_TECHNOLOGY_INFANTRY, infantry_technology_name_count(),
       infantry_technology_name},
  };
  for (size_t catalog = 0; catalog < sizeof(CATALOGS) / sizeof(CATALOGS[0]);
       catalog++) {
    const typeof(*CATALOGS) *entry = checked_storage_at_const(
        CATALOGS, sizeof(CATALOGS) / sizeof(CATALOGS[0]), sizeof(*CATALOGS),
        catalog);
    for (size_t index = 0; index < entry->count; index++) {
      if (strcasecmp(wanted, entry->name_at(index)) == 0) {
        *group = entry->group;
        *bit = (int)(1U << index);
        return true;
      }
    }
  }
  return false;
}

static int technology_set(lua_State *state, LuaBtechPackage *package,
                          bool enabled) {
  Mech *mech = require_mech(state, package);
  if (lua_type(state, 2) != LUA_TSTRING)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "technology must be a string");
  BtechAdminTechnologyGroup group;
  int bit;
  if (!technology_find(lua_tostring(state, 2), &group, &bit))
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "unknown technology");
  if (group == BTECH_ADMIN_TECHNOLOGY_INFANTRY &&
      mech_class(mech) != CLASS_BSUIT)
    return lua_error_arg(state, 1, LUA_ERROR_CODE_OBJECT_INVALID,
                         "infantry technology requires a battlesuit");
  btech_admin_technology_set(mech, group, bit, enabled);
  return 0;
}

static int lua_btech_unit_add_technology(lua_State *state,
                                         LuaBtechPackage *package) {
  return technology_set(state, package, true);
}

static int lua_btech_unit_remove_technology(lua_State *state,
                                            LuaBtechPackage *package) {
  return technology_set(state, package, false);
}

static int lua_btech_unit_clear_technologies(lua_State *state,
                                             LuaBtechPackage *package) {
  Mech *mech = require_mech(state, package);
  if (lua_type(state, 2) != LUA_TSTRING)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "group must be unit, infantry, or all");
  const char *group = lua_tostring(state, 2);
  if (strcasecmp(group, "unit") == 0 || strcasecmp(group, "all") == 0) {
    btech_admin_technologies_clear(mech, BTECH_ADMIN_TECHNOLOGY_PRIMARY);
    btech_admin_technologies_clear(mech, BTECH_ADMIN_TECHNOLOGY_SECONDARY);
  }
  if (strcasecmp(group, "infantry") == 0 || strcasecmp(group, "all") == 0)
    btech_admin_technologies_clear(mech, BTECH_ADMIN_TECHNOLOGY_INFANTRY);
  if (strcasecmp(group, "unit") != 0 && strcasecmp(group, "infantry") != 0 &&
      strcasecmp(group, "all") != 0)
    return lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                         "group must be unit, infantry, or all");
  return 0;
}

static int catalog_index(lua_State *state, int argument, CatalogName *name_at,
                         size_t count, const char *label) {
  if (lua_type(state, argument) != LUA_TSTRING)
    (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                        "%s must be a string", label);
  const char *wanted = lua_tostring(state, argument);
  for (size_t index = 0; index < count; index++)
    if (strcasecmp(wanted, name_at(index)) == 0)
      return (int)index;
  (void)lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID, "unknown %s",
                      label);
  return -1;
}

static int lua_btech_unit_set_unit_type(lua_State *state,
                                        LuaBtechPackage *package) {
  Mech *mech = require_mech(state, package);
  btech_admin_unit_type_set(
      mech, catalog_index(state, 2, template_unit_class_name,
                          template_unit_class_count(), "unit_type"));
  return 0;
}

static int lua_btech_unit_set_movement_type(lua_State *state,
                                            LuaBtechPackage *package) {
  Mech *mech = require_mech(state, package);
  btech_admin_movement_type_set(
      mech, catalog_index(state, 2, template_movement_type_name,
                          template_movement_type_count(), "movement_type"));
  return 0;
}

static int lua_btech_unit_set_jump_speed(lua_State *state,
                                         LuaBtechPackage *package) {
  btech_admin_jump_speed_set(
      require_mech(state, package),
      (float)require_number(state, 2, 0, 10000, "speed"));
  return 0;
}

static int lua_btech_unit_set_heat_sinks(lua_State *state,
                                         LuaBtechPackage *package) {
  btech_admin_heat_sinks_set(require_mech(state, package),
                             require_integer(state, 2, 0, CHAR_MAX, "count"));
  return 0;
}

static int
lua_btech_unit_set_long_range_sensor_range(lua_State *state,
                                           LuaBtechPackage *package) {
  btech_admin_long_range_set(require_mech(state, package),
                             require_integer(state, 2, 0, CHAR_MAX, "range"));
  return 0;
}

static int lua_btech_unit_set_tactical_range(lua_State *state,
                                             LuaBtechPackage *package) {
  btech_admin_tactical_range_set(
      require_mech(state, package),
      require_integer(state, 2, 0, CHAR_MAX, "range"));
  return 0;
}

static int lua_btech_unit_set_scan_range(lua_State *state,
                                         LuaBtechPackage *package) {
  btech_admin_scan_range_set(require_mech(state, package),
                             require_integer(state, 2, 0, CHAR_MAX, "range"));
  return 0;
}

static int lua_btech_unit_set_radio_quality(lua_State *state,
                                            LuaBtechPackage *package) {
  btech_admin_radio_quality_set(require_mech(state, package),
                                require_integer(state, 2, 1, 5, "quality"));
  return 0;
}

static int lua_btech_unit_set_radio_range(lua_State *state,
                                          LuaBtechPackage *package) {
  btech_admin_radio_range_set(require_mech(state, package),
                              require_integer(state, 2, 0, SHRT_MAX, "range"));
  return 0;
}

static int lua_btech_unit_set_cargo_capacity(lua_State *state,
                                             LuaBtechPackage *package) {
  Mech *mech = require_mech(state, package);
  const int SPACE = require_integer(state, 2, 0, 5000, "space");
  const int MAXIMUM_TONS = require_integer(state, 3, 1, 100, "maximum_tons");
  btech_admin_cargo_set(mech, SPACE, MAXIMUM_TONS);
  return 0;
}

static const BtechLuaNativeEntry BTECH_UNIT_ADMIN_ENTRIES[] = {
    {"restore", "unit.restore", lua_btech_unit_restore},
    {"save_template", "unit.save_template", lua_btech_unit_save_template},
    {"reset_critical_slots", "unit.reset_critical_slots",
     lua_btech_unit_reset_critical_slots},
    {"install_weapon", "unit.install_weapon", lua_btech_unit_install_weapon},
    {"configure_ammunition", "unit.configure_ammunition",
     lua_btech_unit_configure_ammunition},
    {"restock_ammunition", "unit.restock_ammunition",
     lua_btech_unit_restock_ammunition},
    {"set_weapon_modes", "unit.set_weapon_modes",
     lua_btech_unit_set_weapon_modes},
    {"install_special", "unit.install_special", lua_btech_unit_install_special},
    {"add_technology", "unit.add_technology", lua_btech_unit_add_technology},
    {"remove_technology", "unit.remove_technology",
     lua_btech_unit_remove_technology},
    {"clear_technologies", "unit.clear_technologies",
     lua_btech_unit_clear_technologies},
    {"set_unit_type", "unit.set_unit_type", lua_btech_unit_set_unit_type},
    {"set_movement_type", "unit.set_movement_type",
     lua_btech_unit_set_movement_type},
    {"set_jump_speed", "unit.set_jump_speed", lua_btech_unit_set_jump_speed},
    {"set_heat_sinks", "unit.set_heat_sinks", lua_btech_unit_set_heat_sinks},
    {"set_long_range_sensor_range", "unit.set_long_range_sensor_range",
     lua_btech_unit_set_long_range_sensor_range},
    {"set_tactical_range", "unit.set_tactical_range",
     lua_btech_unit_set_tactical_range},
    {"set_scan_range", "unit.set_scan_range", lua_btech_unit_set_scan_range},
    {"set_radio_quality", "unit.set_radio_quality",
     lua_btech_unit_set_radio_quality},
    {"set_radio_range", "unit.set_radio_range", lua_btech_unit_set_radio_range},
    {"set_cargo_capacity", "unit.set_cargo_capacity",
     lua_btech_unit_set_cargo_capacity},
};

void lua_btech_install_unit_admin_bindings(lua_State *state,
                                           LuaBtechPackage *package) {
  lua_btech_install_native_bindings(
      state, package, "unit", BTECH_UNIT_ADMIN_ENTRIES,
      sizeof(BTECH_UNIT_ADMIN_ENTRIES) / sizeof(BTECH_UNIT_ADMIN_ENTRIES[0]));
}
