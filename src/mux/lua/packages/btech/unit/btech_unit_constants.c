/* Stable typed constants for BattleTech unit-facing Lua APIs. */

#include "mux/lua/packages/btech/unit/btech_unit_constants.h"

#include "btech/repair/mechrep_api.h"
#include "btech/unit/mech_classification_api.h"
#include "btech/unit/mech_specification_api.h"
#include "btech/unit/mech_utils_api.h"
#include "btech/unit/section_types.h"
#include "btech/unit/template_api.h"
#include "mux/lua/lua_error.h"
#include "mux/lua/lua_error_codes.h"
#include "mux/lua/packages/btech/btech_constants.h"
#include "mux/lua/packages/btech/btech_package.h"
#include "mux/support/checked_storage.h"

#include <lua.h>
#include <stddef.h>

/**
 * @par LuaLS definition btech type btech.unit.constants
 * @code{.lua}
 * ---@class (exact) BtechUnitTypeNamespace
 * ---@field MECH BtechUnitType
 * ---@field VEHICLE BtechUnitType
 * ---@field VTOL BtechUnitType
 * ---@field NAVAL BtechUnitType
 * ---@field SPHEROID_DROPSHIP BtechUnitType
 * ---@field AERO_FIGHTER BtechUnitType
 * ---@field MECHWARRIOR BtechUnitType
 * ---@field AERODYNE_DROPSHIP BtechUnitType
 * ---@field BATTLESUIT BtechUnitType
 * ---@class (exact) BtechMovementTypeNamespace
 * ---@field BIPED BtechMovementType
 * ---@field TRACK BtechMovementType
 * ---@field WHEEL BtechMovementType
 * ---@field HOVER BtechMovementType
 * ---@field VTOL BtechMovementType
 * ---@field HULL BtechMovementType
 * ---@field FOIL BtechMovementType
 * ---@field FLY BtechMovementType
 * ---@field QUAD BtechMovementType
 * ---@field SUB BtechMovementType
 * ---@field NONE BtechMovementType
 * ---@class (exact) BtechSectionNamespace
 * ---@field FRONT_LEFT_LEG BtechSection
 * ---@field FRONT_RIGHT_LEG BtechSection
 * ---@field LEFT_TORSO BtechSection
 * ---@field RIGHT_TORSO BtechSection
 * ---@field CENTER_TORSO BtechSection
 * ---@field REAR_LEFT_LEG BtechSection
 * ---@field REAR_RIGHT_LEG BtechSection
 * ---@field HEAD BtechSection
 * ---@field LEFT_ARM BtechSection
 * ---@field RIGHT_ARM BtechSection
 * ---@field LEFT_LEG BtechSection
 * ---@field RIGHT_LEG BtechSection
 * ---@field SUIT_1 BtechSection
 * ---@field SUIT_2 BtechSection
 * ---@field SUIT_3 BtechSection
 * ---@field SUIT_4 BtechSection
 * ---@field SUIT_5 BtechSection
 * ---@field SUIT_6 BtechSection
 * ---@field SUIT_7 BtechSection
 * ---@field SUIT_8 BtechSection
 * ---@field LEFT_SIDE BtechSection
 * ---@field RIGHT_SIDE BtechSection
 * ---@field FRONT_SIDE BtechSection
 * ---@field AFT_SIDE BtechSection
 * ---@field TURRET BtechSection
 * ---@field ROTOR BtechSection
 * ---@field NOSE BtechSection
 * ---@field LEFT_WING BtechSection
 * ---@field RIGHT_WING BtechSection
 * ---@field LEFT_REAR_WING BtechSection
 * ---@field RIGHT_REAR_WING BtechSection
 * ---@field AFT BtechSection
 * ---@field FRONT_RIGHT_SIDE BtechSection
 * ---@field FRONT_LEFT_SIDE BtechSection
 * ---@field REAR_LEFT_SIDE BtechSection
 * ---@field REAR_RIGHT_SIDE BtechSection
 * ---@class (exact) BtechTechnologyCodeNamespace
 * ---@field TRIPLE_STRENGTH_MYOMER BtechTechnologyCode
 * ---@field CLAN_ANTI_MISSILE BtechTechnologyCode
 * ---@field INNER_SPHERE_ANTI_MISSILE BtechTechnologyCode
 * ---@field DOUBLE_HEAT_SINKS BtechTechnologyCode
 * ---@field MASC BtechTechnologyCode
 * ---@field CLAN BtechTechnologyCode
 * ---@field FLIPPABLE_ARMS BtechTechnologyCode
 * ---@field C3_MASTER BtechTechnologyCode
 * ---@field C3_SLAVE BtechTechnologyCode
 * ---@field ARTEMIS_IV BtechTechnologyCode
 * ---@field ECM BtechTechnologyCode
 * ---@field BEAGLE_PROBE BtechTechnologyCode
 * ---@field SALVAGE BtechTechnologyCode
 * ---@field CARGO BtechTechnologyCode
 * ---@field SEARCH_LIGHT BtechTechnologyCode
 * ---@field LIGHT_ACTIVE_PROBE BtechTechnologyCode
 * ---@field ANTI_AIRCRAFT BtechTechnologyCode
 * ---@field NO_SENSORS BtechTechnologyCode
 * ---@field SIXTH_SENSE BtechTechnologyCode
 * ---@field FERRO_FIBROUS BtechTechnologyCode
 * ---@field ENDO_STEEL BtechTechnologyCode
 * ---@field XL_ENGINE BtechTechnologyCode
 * ---@field ICE_ENGINE BtechTechnologyCode
 * ---@field SINGLE_HEAT_SINKS BtechTechnologyCode
 * ---@field LIGHT_ENGINE BtechTechnologyCode
 * ---@field XXL_ENGINE BtechTechnologyCode
 * ---@field COMPACT_ENGINE BtechTechnologyCode
 * ---@field REINFORCED_INTERNAL BtechTechnologyCode
 * ---@field COMPOSITE_INTERNAL BtechTechnologyCode
 * ---@field HARDENED_ARMOR BtechTechnologyCode
 * ---@field CRITICAL_PROOF BtechTechnologyCode
 * ---@field STEALTH_ARMOR BtechTechnologyCode
 * ---@field HEAVY_FERRO_FIBROUS BtechTechnologyCode
 * ---@field LASER_REFLECTIVE_ARMOR BtechTechnologyCode
 * ---@field REACTIVE_ARMOR BtechTechnologyCode
 * ---@field NULL_SIGNATURE_SYSTEM BtechTechnologyCode
 * ---@field C3I BtechTechnologyCode
 * ---@field SUPERCHARGER BtechTechnologyCode
 * ---@field IMPROVED_JUMP_JETS BtechTechnologyCode
 * ---@field MECHANICAL_JUMP_JETS BtechTechnologyCode
 * ---@field COMPACT_HEAT_SINKS BtechTechnologyCode
 * ---@field LASER_HEAT_SINKS BtechTechnologyCode
 * ---@field BLOODHOUND_PROBE BtechTechnologyCode
 * ---@field ANGEL_ECM BtechTechnologyCode
 * ---@field WATCHDOG BtechTechnologyCode
 * ---@field LIGHT_FERRO_FIBROUS BtechTechnologyCode
 * ---@field TAG BtechTechnologyCode
 * ---@field OMNIMECH BtechTechnologyCode
 * ---@field ARTEMIS_V BtechTechnologyCode
 * ---@field CAMOUFLAGE BtechTechnologyCode
 * ---@field CARRIER BtechTechnologyCode
 * ---@field WATERPROOF BtechTechnologyCode
 * ---@field XL_GYRO BtechTechnologyCode
 * ---@field HEAVY_DUTY_GYRO BtechTechnologyCode
 * ---@field COMPACT_GYRO BtechTechnologyCode
 * ---@field TARGETING_COMPUTER BtechTechnologyCode
 * ---@field SMALL_COCKPIT BtechTechnologyCode
 * ---@field SWARM_ATTACK BtechTechnologyCode
 * ---@field MOUNT_FRIENDS BtechTechnologyCode
 * ---@field ANTI_LEG_ATTACK BtechTechnologyCode
 * ---@field PURIFIER_STEALTH BtechTechnologyCode
 * ---@field KAGE_STEALTH BtechTechnologyCode
 * ---@field ACHILEUS_STEALTH BtechTechnologyCode
 * ---@field INFILTRATOR_STEALTH BtechTechnologyCode
 * ---@field INFILTRATOR_II_STEALTH BtechTechnologyCode
 * ---@field MUST_JETTISON_PACK BtechTechnologyCode
 * ---@field CAN_JETTISON_PACK BtechTechnologyCode
 * ---@class (exact) BtechTechnologyGroupNamespace
 * ---@field UNIT BtechTechnologyGroup
 * ---@field INFANTRY BtechTechnologyGroup
 * ---@field ALL BtechTechnologyGroup
 * ---@class (exact) BtechFireModeNamespace
 * ---@field DESTROYED BtechFireMode
 * ---@field DISABLED BtechFireMode
 * ---@field BROKEN BtechFireMode
 * ---@field DAMAGED BtechFireMode
 * ---@field TARGETING_COMPUTER BtechFireMode
 * ---@field REAR_MOUNT BtechFireMode
 * ---@field HOTLOAD BtechFireMode
 * ---@field HALF_TON BtechFireMode
 * ---@field ONE_SHOT BtechFireMode
 * ---@field ONE_SHOT_USED BtechFireMode
 * ---@field ULTRA BtechFireMode
 * ---@field RAPID_FIRE BtechFireMode
 * ---@field GATLING BtechFireMode
 * ---@field ROTARY_TWO_SHOT BtechFireMode
 * ---@field ROTARY_FOUR_SHOT BtechFireMode
 * ---@field ROTARY_SIX_SHOT BtechFireMode
 * ---@field HEAT BtechFireMode
 * ---@field BACKPACK BtechFireMode
 * ---@field JETTISONED BtechFireMode
 * ---@field OMNI_BASE BtechFireMode
 * ---@field ROCKET_FIRED BtechFireMode
 * ---@class (exact) BtechAmmunitionModeNamespace
 * ---@field LBX_CLUSTER BtechAmmunitionMode
 * ---@field ARTEMIS_MINE BtechAmmunitionMode
 * ---@field NARC_SMOKE BtechAmmunitionMode
 * ---@field CLUSTER BtechAmmunitionMode
 * ---@field MINE BtechAmmunitionMode
 * ---@field SMOKE BtechAmmunitionMode
 * ---@field INFERNO BtechAmmunitionMode
 * ---@field SWARM BtechAmmunitionMode
 * ---@field SWARM_1 BtechAmmunitionMode
 * ---@field INARC_EXPLOSIVE BtechAmmunitionMode
 * ---@field INARC_HAYWIRE BtechAmmunitionMode
 * ---@field INARC_ECM BtechAmmunitionMode
 * ---@field INARC_NEMESIS BtechAmmunitionMode
 * ---@field ARMOR_PIERCING BtechAmmunitionMode
 * ---@field FLECHETTE BtechAmmunitionMode
 * ---@field INCENDIARY BtechAmmunitionMode
 * ---@field PRECISION BtechAmmunitionMode
 * ---@field STINGER BtechAmmunitionMode
 * ---@field CASELESS BtechAmmunitionMode
 * ---@field SEMI_GUIDED BtechAmmunitionMode
 * ---@field EXTENDED_RANGE BtechAmmunitionMode
 * ---@field HIGH_EXPLOSIVE BtechAmmunitionMode
 * ---@field MML_LRM BtechAmmunitionMode
 * @endcode
 */

static const BtechLuaConstantEntry UNIT_TYPES[] = {
    {"MECH", CLASS_MECH},
    {"VEHICLE", CLASS_VEH_GROUND},
    {"VTOL", CLASS_VTOL},
    {"NAVAL", CLASS_VEH_NAVAL},
    {"SPHEROID_DROPSHIP", CLASS_SPHEROID_DS},
    {"AERO_FIGHTER", CLASS_AERO},
    {"MECHWARRIOR", CLASS_MW},
    {"AERODYNE_DROPSHIP", CLASS_DS},
    {"BATTLESUIT", CLASS_BSUIT},
};

const BtechLuaConstantCatalog BTECH_LUA_UNIT_TYPES = {
    .qualified_name = "btech.unit.types",
    .entries = UNIT_TYPES,
    .entry_count = sizeof(UNIT_TYPES) / sizeof(*UNIT_TYPES),
};
static_assert(sizeof(UNIT_TYPES) / sizeof(*UNIT_TYPES) == CLASS_LAST + 1);

static const BtechLuaConstantEntry MOVEMENT_TYPES[] = {
    {"BIPED", MOVE_BIPED}, {"TRACK", MOVE_TRACK}, {"WHEEL", MOVE_WHEEL},
    {"HOVER", MOVE_HOVER}, {"VTOL", MOVE_VTOL},   {"HULL", MOVE_HULL},
    {"FOIL", MOVE_FOIL},   {"FLY", MOVE_FLY},     {"QUAD", MOVE_QUAD},
    {"SUB", MOVE_SUB},     {"NONE", MOVE_NONE},
};

const BtechLuaConstantCatalog BTECH_LUA_MOVEMENT_TYPES = {
    .qualified_name = "btech.unit.movement_types",
    .entries = MOVEMENT_TYPES,
    .entry_count = sizeof(MOVEMENT_TYPES) / sizeof(*MOVEMENT_TYPES),
};
static_assert(sizeof(MOVEMENT_TYPES) / sizeof(*MOVEMENT_TYPES) ==
              MOVE_NONE + 1);

typedef enum BtechLuaSection : int {
  LUA_SECTION_FRONT_LEFT_LEG,
  LUA_SECTION_FRONT_RIGHT_LEG,
  LUA_SECTION_LEFT_TORSO,
  LUA_SECTION_RIGHT_TORSO,
  LUA_SECTION_CENTER_TORSO,
  LUA_SECTION_REAR_LEFT_LEG,
  LUA_SECTION_REAR_RIGHT_LEG,
  LUA_SECTION_HEAD,
  LUA_SECTION_LEFT_ARM,
  LUA_SECTION_RIGHT_ARM,
  LUA_SECTION_LEFT_LEG,
  LUA_SECTION_RIGHT_LEG,
  LUA_SECTION_SUIT_1,
  LUA_SECTION_SUIT_2,
  LUA_SECTION_SUIT_3,
  LUA_SECTION_SUIT_4,
  LUA_SECTION_SUIT_5,
  LUA_SECTION_SUIT_6,
  LUA_SECTION_SUIT_7,
  LUA_SECTION_SUIT_8,
  LUA_SECTION_LEFT_SIDE,
  LUA_SECTION_RIGHT_SIDE,
  LUA_SECTION_FRONT_SIDE,
  LUA_SECTION_AFT_SIDE,
  LUA_SECTION_TURRET,
  LUA_SECTION_ROTOR,
  LUA_SECTION_NOSE,
  LUA_SECTION_LEFT_WING,
  LUA_SECTION_RIGHT_WING,
  LUA_SECTION_LEFT_REAR_WING,
  LUA_SECTION_RIGHT_REAR_WING,
  LUA_SECTION_AFT,
  LUA_SECTION_FRONT_RIGHT_SIDE,
  LUA_SECTION_FRONT_LEFT_SIDE,
  LUA_SECTION_REAR_LEFT_SIDE,
  LUA_SECTION_REAR_RIGHT_SIDE,
  LUA_SECTION_COUNT,
} BtechLuaSection;

static const BtechLuaConstantEntry SECTIONS[] = {
    {"FRONT_LEFT_LEG", LUA_SECTION_FRONT_LEFT_LEG},
    {"FRONT_RIGHT_LEG", LUA_SECTION_FRONT_RIGHT_LEG},
    {"LEFT_TORSO", LUA_SECTION_LEFT_TORSO},
    {"RIGHT_TORSO", LUA_SECTION_RIGHT_TORSO},
    {"CENTER_TORSO", LUA_SECTION_CENTER_TORSO},
    {"REAR_LEFT_LEG", LUA_SECTION_REAR_LEFT_LEG},
    {"REAR_RIGHT_LEG", LUA_SECTION_REAR_RIGHT_LEG},
    {"HEAD", LUA_SECTION_HEAD},
    {"LEFT_ARM", LUA_SECTION_LEFT_ARM},
    {"RIGHT_ARM", LUA_SECTION_RIGHT_ARM},
    {"LEFT_LEG", LUA_SECTION_LEFT_LEG},
    {"RIGHT_LEG", LUA_SECTION_RIGHT_LEG},
    {"SUIT_1", LUA_SECTION_SUIT_1},
    {"SUIT_2", LUA_SECTION_SUIT_2},
    {"SUIT_3", LUA_SECTION_SUIT_3},
    {"SUIT_4", LUA_SECTION_SUIT_4},
    {"SUIT_5", LUA_SECTION_SUIT_5},
    {"SUIT_6", LUA_SECTION_SUIT_6},
    {"SUIT_7", LUA_SECTION_SUIT_7},
    {"SUIT_8", LUA_SECTION_SUIT_8},
    {"LEFT_SIDE", LUA_SECTION_LEFT_SIDE},
    {"RIGHT_SIDE", LUA_SECTION_RIGHT_SIDE},
    {"FRONT_SIDE", LUA_SECTION_FRONT_SIDE},
    {"AFT_SIDE", LUA_SECTION_AFT_SIDE},
    {"TURRET", LUA_SECTION_TURRET},
    {"ROTOR", LUA_SECTION_ROTOR},
    {"NOSE", LUA_SECTION_NOSE},
    {"LEFT_WING", LUA_SECTION_LEFT_WING},
    {"RIGHT_WING", LUA_SECTION_RIGHT_WING},
    {"LEFT_REAR_WING", LUA_SECTION_LEFT_REAR_WING},
    {"RIGHT_REAR_WING", LUA_SECTION_RIGHT_REAR_WING},
    {"AFT", LUA_SECTION_AFT},
    {"FRONT_RIGHT_SIDE", LUA_SECTION_FRONT_RIGHT_SIDE},
    {"FRONT_LEFT_SIDE", LUA_SECTION_FRONT_LEFT_SIDE},
    {"REAR_LEFT_SIDE", LUA_SECTION_REAR_LEFT_SIDE},
    {"REAR_RIGHT_SIDE", LUA_SECTION_REAR_RIGHT_SIDE},
};

const BtechLuaConstantCatalog BTECH_LUA_SECTIONS = {
    .qualified_name = "btech.unit.sections",
    .entries = SECTIONS,
    .entry_count = sizeof(SECTIONS) / sizeof(*SECTIONS),
};
static_assert(sizeof(SECTIONS) / sizeof(*SECTIONS) == LUA_SECTION_COUNT);

typedef enum BtechLuaTechnology : int {
  LUA_TECH_TRIPLE_STRENGTH_MYOMER,
  LUA_TECH_CLAN_ANTI_MISSILE,
  LUA_TECH_INNER_SPHERE_ANTI_MISSILE,
  LUA_TECH_DOUBLE_HEAT_SINKS,
  LUA_TECH_MASC,
  LUA_TECH_CLAN,
  LUA_TECH_FLIPPABLE_ARMS,
  LUA_TECH_C3_MASTER,
  LUA_TECH_C3_SLAVE,
  LUA_TECH_ARTEMIS_IV,
  LUA_TECH_ECM,
  LUA_TECH_BEAGLE_PROBE,
  LUA_TECH_SALVAGE,
  LUA_TECH_CARGO,
  LUA_TECH_SEARCHLIGHT,
  LUA_TECH_LIGHT_ACTIVE_PROBE,
  LUA_TECH_ANTI_AIRCRAFT,
  LUA_TECH_NO_SENSORS,
  LUA_TECH_SIXTH_SENSE,
  LUA_TECH_FERRO_FIBROUS,
  LUA_TECH_ENDO_STEEL,
  LUA_TECH_XL_ENGINE,
  LUA_TECH_ICE_ENGINE,
  LUA_TECH_SINGLE_HEAT_SINKS,
  LUA_TECH_LIGHT_ENGINE,
  LUA_TECH_XXL_ENGINE,
  LUA_TECH_COMPACT_ENGINE,
  LUA_TECH_REINFORCED_INTERNAL,
  LUA_TECH_COMPOSITE_INTERNAL,
  LUA_TECH_HARDENED_ARMOR,
  LUA_TECH_CRITICAL_PROOF,
  LUA_TECH_STEALTH_ARMOR,
  LUA_TECH_HEAVY_FERRO_FIBROUS,
  LUA_TECH_LASER_REFLECTIVE_ARMOR,
  LUA_TECH_REACTIVE_ARMOR,
  LUA_TECH_NULL_SIGNATURE_SYSTEM,
  LUA_TECH_C3I,
  LUA_TECH_SUPERCHARGER,
  LUA_TECH_IMPROVED_JUMP_JETS,
  LUA_TECH_MECHANICAL_JUMP_JETS,
  LUA_TECH_COMPACT_HEAT_SINKS,
  LUA_TECH_LASER_HEAT_SINKS,
  LUA_TECH_BLOODHOUND_PROBE,
  LUA_TECH_ANGEL_ECM,
  LUA_TECH_WATCHDOG,
  LUA_TECH_LIGHT_FERRO_FIBROUS,
  LUA_TECH_TAG,
  LUA_TECH_OMNIMECH,
  LUA_TECH_ARTEMIS_V,
  LUA_TECH_CAMOUFLAGE,
  LUA_TECH_CARRIER,
  LUA_TECH_WATERPROOF,
  LUA_TECH_XL_GYRO,
  LUA_TECH_HEAVY_DUTY_GYRO,
  LUA_TECH_COMPACT_GYRO,
  LUA_TECH_TARGETING_COMPUTER,
  LUA_TECH_SMALL_COCKPIT,
  LUA_TECH_SWARM_ATTACK,
  LUA_TECH_MOUNT_FRIENDS,
  LUA_TECH_ANTI_LEG_ATTACK,
  LUA_TECH_PURIFIER_STEALTH,
  LUA_TECH_KAGE_STEALTH,
  LUA_TECH_ACHILEUS_STEALTH,
  LUA_TECH_INFILTRATOR_STEALTH,
  LUA_TECH_INFILTRATOR_II_STEALTH,
  LUA_TECH_MUST_JETTISON_PACK,
  LUA_TECH_CAN_JETTISON_PACK,
  LUA_TECH_COUNT,
} BtechLuaTechnology;

static const BtechLuaConstantEntry TECHNOLOGY[] = {
    {"TRIPLE_STRENGTH_MYOMER", LUA_TECH_TRIPLE_STRENGTH_MYOMER},
    {"CLAN_ANTI_MISSILE", LUA_TECH_CLAN_ANTI_MISSILE},
    {"INNER_SPHERE_ANTI_MISSILE", LUA_TECH_INNER_SPHERE_ANTI_MISSILE},
    {"DOUBLE_HEAT_SINKS", LUA_TECH_DOUBLE_HEAT_SINKS},
    {"MASC", LUA_TECH_MASC},
    {"CLAN", LUA_TECH_CLAN},
    {"FLIPPABLE_ARMS", LUA_TECH_FLIPPABLE_ARMS},
    {"C3_MASTER", LUA_TECH_C3_MASTER},
    {"C3_SLAVE", LUA_TECH_C3_SLAVE},
    {"ARTEMIS_IV", LUA_TECH_ARTEMIS_IV},
    {"ECM", LUA_TECH_ECM},
    {"BEAGLE_PROBE", LUA_TECH_BEAGLE_PROBE},
    {"SALVAGE", LUA_TECH_SALVAGE},
    {"CARGO", LUA_TECH_CARGO},
    {"SEARCH_LIGHT", LUA_TECH_SEARCHLIGHT},
    {"LIGHT_ACTIVE_PROBE", LUA_TECH_LIGHT_ACTIVE_PROBE},
    {"ANTI_AIRCRAFT", LUA_TECH_ANTI_AIRCRAFT},
    {"NO_SENSORS", LUA_TECH_NO_SENSORS},
    {"SIXTH_SENSE", LUA_TECH_SIXTH_SENSE},
    {"FERRO_FIBROUS", LUA_TECH_FERRO_FIBROUS},
    {"ENDO_STEEL", LUA_TECH_ENDO_STEEL},
    {"XL_ENGINE", LUA_TECH_XL_ENGINE},
    {"ICE_ENGINE", LUA_TECH_ICE_ENGINE},
    {"SINGLE_HEAT_SINKS", LUA_TECH_SINGLE_HEAT_SINKS},
    {"LIGHT_ENGINE", LUA_TECH_LIGHT_ENGINE},
    {"XXL_ENGINE", LUA_TECH_XXL_ENGINE},
    {"COMPACT_ENGINE", LUA_TECH_COMPACT_ENGINE},
    {"REINFORCED_INTERNAL", LUA_TECH_REINFORCED_INTERNAL},
    {"COMPOSITE_INTERNAL", LUA_TECH_COMPOSITE_INTERNAL},
    {"HARDENED_ARMOR", LUA_TECH_HARDENED_ARMOR},
    {"CRITICAL_PROOF", LUA_TECH_CRITICAL_PROOF},
    {"STEALTH_ARMOR", LUA_TECH_STEALTH_ARMOR},
    {"HEAVY_FERRO_FIBROUS", LUA_TECH_HEAVY_FERRO_FIBROUS},
    {"LASER_REFLECTIVE_ARMOR", LUA_TECH_LASER_REFLECTIVE_ARMOR},
    {"REACTIVE_ARMOR", LUA_TECH_REACTIVE_ARMOR},
    {"NULL_SIGNATURE_SYSTEM", LUA_TECH_NULL_SIGNATURE_SYSTEM},
    {"C3I", LUA_TECH_C3I},
    {"SUPERCHARGER", LUA_TECH_SUPERCHARGER},
    {"IMPROVED_JUMP_JETS", LUA_TECH_IMPROVED_JUMP_JETS},
    {"MECHANICAL_JUMP_JETS", LUA_TECH_MECHANICAL_JUMP_JETS},
    {"COMPACT_HEAT_SINKS", LUA_TECH_COMPACT_HEAT_SINKS},
    {"LASER_HEAT_SINKS", LUA_TECH_LASER_HEAT_SINKS},
    {"BLOODHOUND_PROBE", LUA_TECH_BLOODHOUND_PROBE},
    {"ANGEL_ECM", LUA_TECH_ANGEL_ECM},
    {"WATCHDOG", LUA_TECH_WATCHDOG},
    {"LIGHT_FERRO_FIBROUS", LUA_TECH_LIGHT_FERRO_FIBROUS},
    {"TAG", LUA_TECH_TAG},
    {"OMNIMECH", LUA_TECH_OMNIMECH},
    {"ARTEMIS_V", LUA_TECH_ARTEMIS_V},
    {"CAMOUFLAGE", LUA_TECH_CAMOUFLAGE},
    {"CARRIER", LUA_TECH_CARRIER},
    {"WATERPROOF", LUA_TECH_WATERPROOF},
    {"XL_GYRO", LUA_TECH_XL_GYRO},
    {"HEAVY_DUTY_GYRO", LUA_TECH_HEAVY_DUTY_GYRO},
    {"COMPACT_GYRO", LUA_TECH_COMPACT_GYRO},
    {"TARGETING_COMPUTER", LUA_TECH_TARGETING_COMPUTER},
    {"SMALL_COCKPIT", LUA_TECH_SMALL_COCKPIT},
    {"SWARM_ATTACK", LUA_TECH_SWARM_ATTACK},
    {"MOUNT_FRIENDS", LUA_TECH_MOUNT_FRIENDS},
    {"ANTI_LEG_ATTACK", LUA_TECH_ANTI_LEG_ATTACK},
    {"PURIFIER_STEALTH", LUA_TECH_PURIFIER_STEALTH},
    {"KAGE_STEALTH", LUA_TECH_KAGE_STEALTH},
    {"ACHILEUS_STEALTH", LUA_TECH_ACHILEUS_STEALTH},
    {"INFILTRATOR_STEALTH", LUA_TECH_INFILTRATOR_STEALTH},
    {"INFILTRATOR_II_STEALTH", LUA_TECH_INFILTRATOR_II_STEALTH},
    {"MUST_JETTISON_PACK", LUA_TECH_MUST_JETTISON_PACK},
    {"CAN_JETTISON_PACK", LUA_TECH_CAN_JETTISON_PACK},
};

const BtechLuaConstantCatalog BTECH_LUA_TECHNOLOGY = {
    .qualified_name = "btech.unit.technology",
    .entries = TECHNOLOGY,
    .entry_count = sizeof(TECHNOLOGY) / sizeof(*TECHNOLOGY),
};
static_assert(sizeof(TECHNOLOGY) / sizeof(*TECHNOLOGY) == LUA_TECH_COUNT);

static const BtechLuaConstantEntry TECHNOLOGY_GROUPS[] = {
    {"UNIT", BTECH_LUA_TECHNOLOGY_GROUP_UNIT},
    {"INFANTRY", BTECH_LUA_TECHNOLOGY_GROUP_INFANTRY},
    {"ALL", BTECH_LUA_TECHNOLOGY_GROUP_ALL},
};

const BtechLuaConstantCatalog BTECH_LUA_TECHNOLOGY_GROUPS = {
    .qualified_name = "btech.unit.technology_groups",
    .entries = TECHNOLOGY_GROUPS,
    .entry_count = sizeof(TECHNOLOGY_GROUPS) / sizeof(*TECHNOLOGY_GROUPS),
};

static const BtechLuaConstantEntry FIRE_MODE_CONSTANTS[] = {
    {"DESTROYED", DESTROYED_MODE},
    {"DISABLED", DISABLED_MODE},
    {"BROKEN", BROKEN_MODE},
    {"DAMAGED", DAMAGED_MODE},
    {"TARGETING_COMPUTER", ON_TC},
    {"REAR_MOUNT", REAR_MOUNT},
    {"HOTLOAD", HOTLOAD_MODE},
    {"HALF_TON", HALFTON_MODE},
    {"ONE_SHOT", OS_MODE},
    {"ONE_SHOT_USED", OS_USED},
    {"ULTRA", ULTRA_MODE},
    {"RAPID_FIRE", RFAC_MODE},
    {"GATLING", GATTLING_MODE},
    {"ROTARY_TWO_SHOT", RAC_TWOSHOT_MODE},
    {"ROTARY_FOUR_SHOT", RAC_FOURSHOT_MODE},
    {"ROTARY_SIX_SHOT", RAC_SIXSHOT_MODE},
    {"HEAT", HEAT_MODE},
    {"BACKPACK", WILL_JETTISON_MODE},
    {"JETTISONED", IS_JETTISONED_MODE},
    {"OMNI_BASE", OMNI_BASE_MODE},
    {"ROCKET_FIRED", ROCKET_FIRED},
};

const BtechLuaConstantCatalog BTECH_LUA_FIRE_MODES = {
    .qualified_name = "btech.unit.fire_modes",
    .entries = FIRE_MODE_CONSTANTS,
    .entry_count = sizeof(FIRE_MODE_CONSTANTS) / sizeof(*FIRE_MODE_CONSTANTS),
};
static_assert(sizeof(FIRE_MODE_CONSTANTS) / sizeof(*FIRE_MODE_CONSTANTS) == 21);

static const BtechLuaConstantEntry AMMUNITION_MODE_CONSTANTS[] = {
    {"LBX_CLUSTER", LBX_MODE},
    {"ARTEMIS_MINE", ARTEMIS_MODE},
    {"NARC_SMOKE", NARC_MODE},
    {"CLUSTER", CLUSTER_MODE},
    {"MINE", MINE_MODE},
    {"SMOKE", SMOKE_MODE},
    {"INFERNO", INFERNO_MODE},
    {"SWARM", SWARM_MODE},
    {"SWARM_1", SWARM1_MODE},
    {"INARC_EXPLOSIVE", INARC_EXPLO_MODE},
    {"INARC_HAYWIRE", INARC_HAYWIRE_MODE},
    {"INARC_ECM", INARC_ECM_MODE},
    {"INARC_NEMESIS", INARC_NEMESIS_MODE},
    {"ARMOR_PIERCING", AC_AP_MODE},
    {"FLECHETTE", AC_FLECHETTE_MODE},
    {"INCENDIARY", AC_INCENDIARY_MODE},
    {"PRECISION", AC_PRECISION_MODE},
    {"STINGER", STINGER_MODE},
    {"CASELESS", AC_CASELESS_MODE},
    {"SEMI_GUIDED", SGUIDED_MODE},
    {"EXTENDED_RANGE", ATM_ER_MODE},
    {"HIGH_EXPLOSIVE", ATM_HE_MODE},
    {"MML_LRM", MML_LRM_MODE},
};

const BtechLuaConstantCatalog BTECH_LUA_AMMUNITION_MODES = {
    .qualified_name = "btech.unit.ammunition_modes",
    .entries = AMMUNITION_MODE_CONSTANTS,
    .entry_count =
        sizeof(AMMUNITION_MODE_CONSTANTS) / sizeof(*AMMUNITION_MODE_CONSTANTS),
};
static_assert(sizeof(AMMUNITION_MODE_CONSTANTS) /
                  sizeof(*AMMUNITION_MODE_CONSTANTS) ==
              23);

static int section_value(lua_State *state, const Mech *mech, int section) {
  const UnitSectionCatalog CATALOG = {
      .unit_type = mech_class(mech), .movement_type = mech_movement_type(mech)};
  if (section < 0 || (size_t)section >= unit_section_name_count(&CATALOG)) {
    (void)lua_error_raise(state, LUA_ERROR_CODE_INTERNAL,
                          "unit contains an invalid section");
    return -1;
  }
  switch (mech_class(mech)) {
  case CLASS_BSUIT:
    return LUA_SECTION_SUIT_1 + section;
  case CLASS_MECH:
  case CLASS_MW:
    if (mech_movement_type(mech) == MOVE_QUAD) {
      static const int QUAD[] = {
          LUA_SECTION_FRONT_LEFT_LEG, LUA_SECTION_FRONT_RIGHT_LEG,
          LUA_SECTION_LEFT_TORSO,     LUA_SECTION_RIGHT_TORSO,
          LUA_SECTION_CENTER_TORSO,   LUA_SECTION_REAR_LEFT_LEG,
          LUA_SECTION_REAR_RIGHT_LEG, LUA_SECTION_HEAD,
      };
      return *(const int *)checked_storage_at_const(
          QUAD, sizeof(QUAD) / sizeof(*QUAD), sizeof(*QUAD), (size_t)section);
    }
    {
      static const int MECH[] = {
          LUA_SECTION_LEFT_ARM,     LUA_SECTION_RIGHT_ARM,
          LUA_SECTION_LEFT_TORSO,   LUA_SECTION_RIGHT_TORSO,
          LUA_SECTION_CENTER_TORSO, LUA_SECTION_LEFT_LEG,
          LUA_SECTION_RIGHT_LEG,    LUA_SECTION_HEAD,
      };
      return *(const int *)checked_storage_at_const(
          MECH, sizeof(MECH) / sizeof(*MECH), sizeof(*MECH), (size_t)section);
    }
  case CLASS_VEH_GROUND:
  case CLASS_VEH_NAVAL:
  case CLASS_VTOL: {
    static const int VEHICLE[] = {
        LUA_SECTION_LEFT_SIDE, LUA_SECTION_RIGHT_SIDE, LUA_SECTION_FRONT_SIDE,
        LUA_SECTION_AFT_SIDE,  LUA_SECTION_TURRET,     LUA_SECTION_ROTOR,
    };
    return *(const int *)checked_storage_at_const(
        VEHICLE, sizeof(VEHICLE) / sizeof(*VEHICLE), sizeof(*VEHICLE),
        (size_t)section);
  }
  case CLASS_AERO: {
    static const int AERO[] = {LUA_SECTION_NOSE, LUA_SECTION_LEFT_WING,
                               LUA_SECTION_RIGHT_WING, LUA_SECTION_AFT_SIDE};
    return *(const int *)checked_storage_at_const(
        AERO, sizeof(AERO) / sizeof(*AERO), sizeof(*AERO), (size_t)section);
  }
  case CLASS_DS: {
    static const int DROPSHIP[] = {
        LUA_SECTION_RIGHT_WING,
        LUA_SECTION_LEFT_WING,
        LUA_SECTION_LEFT_REAR_WING,
        LUA_SECTION_RIGHT_REAR_WING,
        LUA_SECTION_AFT,
        LUA_SECTION_NOSE,
    };
    return *(const int *)checked_storage_at_const(
        DROPSHIP, sizeof(DROPSHIP) / sizeof(*DROPSHIP), sizeof(*DROPSHIP),
        (size_t)section);
  }
  case CLASS_SPHEROID_DS: {
    static const int SPHEROID[] = {
        LUA_SECTION_FRONT_RIGHT_SIDE,
        LUA_SECTION_FRONT_LEFT_SIDE,
        LUA_SECTION_REAR_LEFT_SIDE,
        LUA_SECTION_REAR_RIGHT_SIDE,
        LUA_SECTION_AFT,
        LUA_SECTION_NOSE,
    };
    return *(const int *)checked_storage_at_const(
        SPHEROID, sizeof(SPHEROID) / sizeof(*SPHEROID), sizeof(*SPHEROID),
        (size_t)section);
  }
  }
  (void)lua_error_raise(state, LUA_ERROR_CODE_INTERNAL,
                        "unit has an unsupported class");
  return -1;
}

void lua_btech_section_push(lua_State *state, LuaBtechPackage *package,
                            const Mech *mech, int section) {
  lua_btech_constant_push(state, package, &BTECH_LUA_SECTIONS,
                          section_value(state, mech, section));
}

int lua_btech_section_require_at(LuaBtechPackage *package, lua_State *state,
                                 int index, int argument, const char *label,
                                 const Mech *mech) {
  const int VALUE = lua_btech_constant_require_at(
      package, state, index, argument, label, &BTECH_LUA_SECTIONS);
  const int COUNT = (int)unit_section_name_count(
      &(UnitSectionCatalog){.unit_type = mech_class(mech),
                            .movement_type = mech_movement_type(mech)});
  for (int section = 0; section < COUNT; section++)
    if (section_value(state, mech, section) == VALUE)
      return section;
  return lua_error_arg(state, argument, LUA_ERROR_CODE_ARG_INVALID,
                       "%s is not valid for this unit", label);
}

static const BtechLuaTechnologySelection TECHNOLOGY_SELECTIONS[] = {
    [LUA_TECH_TRIPLE_STRENGTH_MYOMER] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY,
                                         1U << 0},
    [LUA_TECH_CLAN_ANTI_MISSILE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 1},
    [LUA_TECH_INNER_SPHERE_ANTI_MISSILE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY,
                                            1U << 2},
    [LUA_TECH_DOUBLE_HEAT_SINKS] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 3},
    [LUA_TECH_MASC] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 4},
    [LUA_TECH_CLAN] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 5},
    [LUA_TECH_FLIPPABLE_ARMS] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 6},
    [LUA_TECH_C3_MASTER] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 7},
    [LUA_TECH_C3_SLAVE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 8},
    [LUA_TECH_ARTEMIS_IV] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 9},
    [LUA_TECH_ECM] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 10},
    [LUA_TECH_BEAGLE_PROBE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 11},
    [LUA_TECH_SALVAGE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 12},
    [LUA_TECH_CARGO] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 13},
    [LUA_TECH_SEARCHLIGHT] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 14},
    [LUA_TECH_LIGHT_ACTIVE_PROBE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 15},
    [LUA_TECH_ANTI_AIRCRAFT] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 16},
    [LUA_TECH_NO_SENSORS] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 17},
    [LUA_TECH_SIXTH_SENSE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 18},
    [LUA_TECH_FERRO_FIBROUS] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 19},
    [LUA_TECH_ENDO_STEEL] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 20},
    [LUA_TECH_XL_ENGINE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 21},
    [LUA_TECH_ICE_ENGINE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 22},
    [LUA_TECH_SINGLE_HEAT_SINKS] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 23},
    [LUA_TECH_LIGHT_ENGINE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 24},
    [LUA_TECH_XXL_ENGINE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 25},
    [LUA_TECH_COMPACT_ENGINE] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 26},
    [LUA_TECH_REINFORCED_INTERNAL] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 27},
    [LUA_TECH_COMPOSITE_INTERNAL] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 28},
    [LUA_TECH_HARDENED_ARMOR] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 29},
    [LUA_TECH_CRITICAL_PROOF] = {BTECH_ADMIN_TECHNOLOGY_PRIMARY, 1U << 30},
    [LUA_TECH_STEALTH_ARMOR] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 0},
    [LUA_TECH_HEAVY_FERRO_FIBROUS] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY,
                                      1U << 1},
    [LUA_TECH_LASER_REFLECTIVE_ARMOR] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY,
                                         1U << 2},
    [LUA_TECH_REACTIVE_ARMOR] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 3},
    [LUA_TECH_NULL_SIGNATURE_SYSTEM] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY,
                                        1U << 4},
    [LUA_TECH_C3I] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 5},
    [LUA_TECH_SUPERCHARGER] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 6},
    [LUA_TECH_IMPROVED_JUMP_JETS] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 7},
    [LUA_TECH_MECHANICAL_JUMP_JETS] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY,
                                       1U << 8},
    [LUA_TECH_COMPACT_HEAT_SINKS] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 9},
    [LUA_TECH_LASER_HEAT_SINKS] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 10},
    [LUA_TECH_BLOODHOUND_PROBE] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 11},
    [LUA_TECH_ANGEL_ECM] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 12},
    [LUA_TECH_WATCHDOG] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 13},
    [LUA_TECH_LIGHT_FERRO_FIBROUS] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY,
                                      1U << 14},
    [LUA_TECH_TAG] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 15},
    [LUA_TECH_OMNIMECH] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 16},
    [LUA_TECH_ARTEMIS_V] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 17},
    [LUA_TECH_CAMOUFLAGE] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 18},
    [LUA_TECH_CARRIER] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 19},
    [LUA_TECH_WATERPROOF] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 20},
    [LUA_TECH_XL_GYRO] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 21},
    [LUA_TECH_HEAVY_DUTY_GYRO] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 22},
    [LUA_TECH_COMPACT_GYRO] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 23},
    [LUA_TECH_TARGETING_COMPUTER] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY,
                                     1U << 24},
    [LUA_TECH_SMALL_COCKPIT] = {BTECH_ADMIN_TECHNOLOGY_SECONDARY, 1U << 25},
    [LUA_TECH_SWARM_ATTACK] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 0},
    [LUA_TECH_MOUNT_FRIENDS] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 1},
    [LUA_TECH_ANTI_LEG_ATTACK] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 2},
    [LUA_TECH_PURIFIER_STEALTH] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 3},
    [LUA_TECH_KAGE_STEALTH] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 4},
    [LUA_TECH_ACHILEUS_STEALTH] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 5},
    [LUA_TECH_INFILTRATOR_STEALTH] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 6},
    [LUA_TECH_INFILTRATOR_II_STEALTH] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY,
                                         1U << 7},
    [LUA_TECH_MUST_JETTISON_PACK] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 8},
    [LUA_TECH_CAN_JETTISON_PACK] = {BTECH_ADMIN_TECHNOLOGY_INFANTRY, 1U << 9},
};
static_assert(sizeof(TECHNOLOGY_SELECTIONS) / sizeof(*TECHNOLOGY_SELECTIONS) ==
              LUA_TECH_COUNT);

BtechLuaTechnologySelection lua_btech_technology_selection(lua_State *state,
                                                           int value) {
  if (value < 0 || value >= LUA_TECH_COUNT) {
    (void)lua_error_raise(state, LUA_ERROR_CODE_INTERNAL,
                          "unknown technology constant value");
    return (BtechLuaTechnologySelection){};
  }
  return *(const BtechLuaTechnologySelection *)checked_storage_at_const(
      TECHNOLOGY_SELECTIONS,
      sizeof(TECHNOLOGY_SELECTIONS) / sizeof(*TECHNOLOGY_SELECTIONS),
      sizeof(*TECHNOLOGY_SELECTIONS), (size_t)value);
}

void lua_btech_unit_constants_install(lua_State *state,
                                      LuaBtechPackage *package) {
  if (BTECH_LUA_UNIT_TYPES.entry_count != template_unit_class_count() ||
      BTECH_LUA_MOVEMENT_TYPES.entry_count != template_movement_type_count() ||
      BTECH_LUA_TECHNOLOGY.entry_count !=
          primary_technology_name_count() + secondary_technology_name_count() +
              infantry_technology_name_count() ||
      BTECH_LUA_FIRE_MODES.entry_count != template_critical_fire_mode_count() ||
      BTECH_LUA_AMMUNITION_MODES.entry_count !=
          template_critical_ammo_mode_count()) {
    (void)lua_error_raise(state, LUA_ERROR_CODE_INTERNAL,
                          "BTech constant inventory is out of sync");
    return;
  }
  lua_btech_constant_install_namespace(state, package, "types",
                                       &BTECH_LUA_UNIT_TYPES);
  lua_btech_constant_install_namespace(state, package, "movement_types",
                                       &BTECH_LUA_MOVEMENT_TYPES);
  lua_btech_constant_install_namespace(state, package, "sections",
                                       &BTECH_LUA_SECTIONS);
  lua_btech_constant_install_namespace(state, package, "technology",
                                       &BTECH_LUA_TECHNOLOGY);
  lua_btech_constant_install_namespace(state, package, "technology_groups",
                                       &BTECH_LUA_TECHNOLOGY_GROUPS);
  lua_btech_constant_install_namespace(state, package, "fire_modes",
                                       &BTECH_LUA_FIRE_MODES);
  lua_btech_constant_install_namespace(state, package, "ammunition_modes",
                                       &BTECH_LUA_AMMUNITION_MODES);
}
