#include "mechrep_api.h"

#undef NDEBUG
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "btech/context.h"
#include "command_handlers_api.h"
#include "equipment_types.h"
#include "mech_build_api.h"
#include "mech_classification_api.h"
#include "mech_equipment_api.h"
#include "mech_identity_api.h"
#include "mech_specification_api.h"
#include "mech_status_types.h"
#include "mech_utils_api.h"
#include "mux/network/network_output.h"
#include "mux/support/checked_storage.h"
#include "registry_api.h"
#include "repair_job.h"
#include "template_api.h"
#include "weapon_catalogue_api.h"

static BtechContext *const context = (BtechContext *)1;
static EvaluationContext *const evaluation = (EvaluationContext *)2;
static Mech *const mech = (Mech *)3;
static RepairCommandStatus command_status;
static UnitClass unit_class;
static int technology_flags;
static int secondary_technology_flags;
static int infantry_flags;
static int cargo_space;
static int carrier_maximum_tonnage;
static int section_critical_count;
static int weapon_critical_count;
static int ammunition_per_ton_calls;
static int full_ammo_calls;
static int data_set_calls;
static int critical_part_type_calls;
static int case_configuration_removals;
static int section_configurations[NUM_SECTIONS];
static bool critical_destroyed;
static int configured_count;
static CriticalSlotConfiguration configured[NUM_CRITICALS];
static bool use_critical_grid;
enum { CRITICAL_PART_CAPACITY = NUM_SECTIONS * NUM_CRITICALS };
static int critical_parts[CRITICAL_PART_CAPACITY];
static struct {
  int part_type;
  int data;
  int fire_mode;
  int ammo_mode;
  int damage_flags;
  int temporary_failure;
  int brand;
  int desired_ammo_section;
} special_slot;

static int *critical_part_address(int section, int critical) {
  return checked_storage_at(
      critical_parts, CRITICAL_PART_CAPACITY, sizeof(*critical_parts),
      ((size_t)section * (size_t)NUM_CRITICALS) + (size_t)critical);
}

static int *section_configuration_address(int section) {
  return checked_storage_at(section_configurations, NUM_SECTIONS,
                            sizeof(*section_configurations), (size_t)section);
}

static void reset_state(void) {
  command_status = REPAIR_COMMAND_READY;
  unit_class = CLASS_BSUIT;
  technology_flags = 0;
  secondary_technology_flags = 0;
  infantry_flags = 123;
  cargo_space = 456;
  carrier_maximum_tonnage = 78;
  section_critical_count = 2;
  weapon_critical_count = 2;
  ammunition_per_ton_calls = 0;
  full_ammo_calls = 0;
  data_set_calls = 0;
  critical_part_type_calls = 0;
  case_configuration_removals = 0;
  memset(section_configurations, 0, sizeof(section_configurations));
  critical_destroyed = false;
  configured_count = 0;
  use_critical_grid = false;
  memset(configured, 0, sizeof(configured));
  memset(critical_parts, 0, sizeof(critical_parts));
  special_slot = (typeof(special_slot)){
      .part_type = 901,
      .data = 19,
      .fire_mode = DESTROYED_MODE | DISABLED_MODE | DAMAGED_MODE,
      .ammo_mode = ARTEMIS_MODE,
      .damage_flags = WEAP_DAM_MODERATE,
      .temporary_failure = 15,
      .brand = 9,
      .desired_ammo_section = RTORSO,
  };
}

RepairCommandStatus
mech_admin_command_context_initialize(DbRef player, void *data,
                                      MechAdminCommandContext *command) {
  assert(data == mech);
  *command = (MechAdminCommandContext){.player = player,
                                       .context = context,
                                       .evaluation = evaluation,
                                       .mech = mech};
  return command_status;
}

const char *repair_command_status_message(RepairCommandStatus status
                                          [[maybe_unused]]) {
  return "Unavailable.";
}

EvaluationContext *btech_context_evaluation(BtechContext *value) {
  return value == context ? evaluation : nullptr;
}

BtechContext *mech_context(const Mech *value [[maybe_unused]]) {
  return context;
}

UnitClass mech_class(const Mech *value [[maybe_unused]]) { return unit_class; }

MechMovementType mech_movement_type(const Mech *value [[maybe_unused]]) {
  return MOVE_BIPED;
}

void mech_infantry_technology_flags_set(Mech *value [[maybe_unused]],
                                        int flags) {
  infantry_flags = flags;
}

void mech_cargo_space_set(Mech *value [[maybe_unused]], int space) {
  cargo_space = space;
}

void mech_carrier_maximum_tonnage_set(Mech *value [[maybe_unused]],
                                      int tonnage) {
  carrier_maximum_tonnage = tonnage;
}

int mech_parseattributes(char *buffer, char **arguments, int maxargs) {
  int count = 0;
  for (char *token = strtok(buffer, " "); token != nullptr && count < maxargs;
       token = strtok(nullptr, " "))
    *(char **)checked_storage_at(arguments, (size_t)maxargs, sizeof(*arguments),
                                 (size_t)count++) = token;
  return count;
}

int bounded(int minimum, int value, int maximum) {
  if (value < minimum)
    return minimum;
  if (value > maximum)
    return maximum;
  return value;
}

int armor_section_from_string(UnitClass type [[maybe_unused]],
                              MechMovementType movement [[maybe_unused]],
                              const char *section) {
  return strcmp(section, "HEAD") == 0 ? HEAD : -1;
}

int weapon_index_from_string(BtechContext *value, const char *name) {
  return value == context && strcmp(name, "Laser") == 0 ? 7 : -1;
}

int get_weapon_crits(Mech *value [[maybe_unused]],
                     int weapon_index [[maybe_unused]]) {
  return weapon_critical_count;
}

int mech_section_critical_count(Mech *value [[maybe_unused]],
                                int section [[maybe_unused]]) {
  return section_critical_count;
}

void mech_critical_configure(const CriticalSlotConfiguration *configuration) {
  *(CriticalSlotConfiguration *)checked_storage_at(
      configured, NUM_CRITICALS, sizeof(*configured),
      (size_t)configured_count++) = *configuration;
  special_slot.part_type = configuration->part_type;
  special_slot.data = configuration->data;
  special_slot.fire_mode = configuration->fire_mode;
  special_slot.ammo_mode = configuration->ammo_mode;
  if (use_critical_grid)
    *critical_part_address(configuration->slot.section,
                           configuration->slot.critical) =
        configuration->part_type;
}

void mech_critical_damage_flags_set(Mech *value [[maybe_unused]],
                                    int section [[maybe_unused]],
                                    int critical [[maybe_unused]], int flags) {
  special_slot.damage_flags = flags;
}

void mech_critical_temporary_failure_set(
    const CriticalSlotFailureSet *request) {
  special_slot.temporary_failure = request->failure;
}

void mech_critical_brand_set(const CriticalSlotBrandSet *request) {
  special_slot.brand = request->brand;
}

void mech_critical_desired_ammo_section_set(Mech *value [[maybe_unused]],
                                            int section [[maybe_unused]],
                                            int critical [[maybe_unused]],
                                            int ammo_section) {
  special_slot.desired_ammo_section = ammo_section;
}

int find_special_item_code_from_string(BtechContext *value, const char *name) {
  return value == context && strcmp(name, "Widget") == 0 ? 77 : -1;
}

void armor_string_from_index(int section [[maybe_unused]], char *buffer,
                             UnitClass type [[maybe_unused]],
                             MechMovementType movement [[maybe_unused]]) {
  strcpy(buffer, "HEAD");
}

bool weapon_catalogue_has_special(int weapon_index [[maybe_unused]],
                                  int special [[maybe_unused]]) {
  return false;
}

bool weapon_catalogue_is_anti_missile(int weapon_index [[maybe_unused]]) {
  return false;
}

void mech_technology_flags_add(Mech *value [[maybe_unused]], int flags) {
  technology_flags |= flags;
}

void mech_technology_flags_remove(Mech *value [[maybe_unused]], int flags) {
  technology_flags &= ~flags;
}

int mech_technology_flags(const Mech *value [[maybe_unused]]) {
  return technology_flags;
}

void mech_technology_flags_secondary_add(Mech *value [[maybe_unused]],
                                         int flags) {
  secondary_technology_flags |= flags;
}

void mech_technology_flags_secondary_remove(Mech *value [[maybe_unused]],
                                            int flags) {
  secondary_technology_flags &= ~flags;
}

void mech_infantry_technology_flags_add(Mech *value [[maybe_unused]],
                                        int flags) {
  infantry_flags |= flags;
}

int mech_infantry_technology_flags(const Mech *value [[maybe_unused]]) {
  return infantry_flags;
}

void mech_section_configuration_add(Mech *value [[maybe_unused]], int section,
                                    int configuration) {
  *section_configuration_address(section) |= configuration;
}

void mech_section_configuration_remove(Mech *value [[maybe_unused]],
                                       int section [[maybe_unused]],
                                       int configuration) {
  assert(configuration == CASE_TECH);
  *section_configuration_address(section) &= ~configuration;
  case_configuration_removals++;
}

int crits_in_loc(Mech *value [[maybe_unused]], int section [[maybe_unused]]) {
  return section_critical_count;
}

int mech_critical_data(const Mech *value [[maybe_unused]],
                       int section [[maybe_unused]],
                       int critical [[maybe_unused]]) {
  return special_slot.data;
}

void mech_critical_data_set(Mech *value [[maybe_unused]],
                            int section [[maybe_unused]],
                            int critical [[maybe_unused]], int data) {
  data_set_calls++;
  special_slot.data = data;
}

int mech_critical_part_type(const Mech *value [[maybe_unused]], int section,
                            int critical) {
  critical_part_type_calls++;
  if (use_critical_grid)
    return *critical_part_address(section, critical);
  return special_slot.part_type;
}

void mech_critical_part_type_set(Mech *value [[maybe_unused]], int section,
                                 int critical, int part_type) {
  assert(use_critical_grid);
  *critical_part_address(section, critical) = part_type;
}

bool mech_critical_is_destroyed(const Mech *value [[maybe_unused]],
                                int section [[maybe_unused]],
                                int critical [[maybe_unused]]) {
  return critical_destroyed;
}

int weapon_catalogue_ammunition_per_ton(int weapon_index) {
  ammunition_per_ton_calls++;
  return weapon_index == 7 ? 20 : 0;
}

int full_ammo(const Mech *value [[maybe_unused]], int section [[maybe_unused]],
              int critical [[maybe_unused]]) {
  full_ammo_calls++;
  return 42;
}

void dump_mech_special_objects(BtechContext *value [[maybe_unused]],
                               DbRef player [[maybe_unused]]) {}

void dump_weapons(BtechContext *value [[maybe_unused]],
                  DbRef player [[maybe_unused]]) {}

void mecha_notify(EvaluationContext *value [[maybe_unused]],
                  DbRef player [[maybe_unused]],
                  const char *message [[maybe_unused]]) {}

void notify_printf(EvaluationContext *value [[maybe_unused]],
                   DbRef player [[maybe_unused]],
                   const char *format [[maybe_unused]], ...) {}

static void test_delinftech_uses_repair_target(void) {
  reset_state();
  mechrep_rdelinftech(99, mech, nullptr);
  assert(infantry_flags == 0);

  reset_state();
  command_status = REPAIR_COMMAND_MISSING_MECH;
  mechrep_rdelinftech(99, mech, nullptr);
  assert(infantry_flags == 123);

  reset_state();
  command_status = REPAIR_COMMAND_UNAUTHORIZED;
  mechrep_rdelinftech(99, mech, nullptr);
  assert(infantry_flags == 123);

  reset_state();
  unit_class = CLASS_MECH;
  mechrep_rdelinftech(99, mech, nullptr);
  assert(infantry_flags == 123);
}

static void assert_no_addition(const char *input) {
  char command[64];

  strcpy(command, input);
  mechrep_raddweap(99, mech, command);
  assert(configured_count == 0);
}

static void test_addweap_validates_before_mutation(void) {
  char valid[] = "Laser HEAD 1 2";
  char split_valid[] = "Laser HEAD 1 2";

  reset_state();
  mechrep_raddweap(99, mech, valid);
  assert(configured_count == 2);
  assert(configured[0].slot.section == HEAD);
  assert(configured[0].slot.critical == 0);
  assert(configured[1].slot.critical == 1);

  reset_state();
  use_critical_grid = true;
  technology_flags = ECM_TECH;
  *critical_part_address(HEAD, 0) = special_equipment_index(ECM);
  assert_no_addition("Laser HEAD 1");
  assert_no_addition("Laser HEAD 1 2 3");
  assert_no_addition("Laser HEAD 13 1");
  assert_no_addition("Laser HEAD 1 13");
  assert_no_addition("Laser HEAD 1 1");
  assert((technology_flags & ECM_TECH) != 0);
  assert(*critical_part_address(HEAD, 0) == special_equipment_index(ECM));

  reset_state();
  weapon_critical_count = 9;
  mechrep_raddweap(99, mech, split_valid);
  assert(configured_count == 2);
}

static void test_raddspecial_replaces_slot_with_fresh_state(void) {
  char command[] = "Widget HEAD 1 47";

  reset_state();
  mechrep_raddspecial(99, mech, command);
  assert(configured_count == 1);
  assert(configured[0].slot.section == HEAD);
  assert(configured[0].slot.critical == 0);
  assert(special_slot.part_type == special_equipment_index(77));
  assert(special_slot.data == 47);
  assert(special_slot.fire_mode == 0);
  assert(special_slot.ammo_mode == 0);
  assert(special_slot.damage_flags == 0);
  assert(special_slot.temporary_failure == 0);
  assert(special_slot.brand == 0);
  assert(special_slot.desired_ammo_section == -1);
}

typedef struct SpecialTechnologyExpectation {
  int special;
  int flag;
  bool secondary;
} SpecialTechnologyExpectation;

static void test_special_install_reconciles_every_derived_technology(void) {
  static const SpecialTechnologyExpectation EXPECTATIONS[] = {
      {TRIPLE_STRENGTH_MYOMER, TRIPLE_MYOMER_TECH, false},
      {MASC, MASC_TECH, false},
      {C3_MASTER, C3_MASTER_TECH, false},
      {C3_SLAVE, C3_SLAVE_TECH, false},
      {ARTEMIS_IV, ARTEMIS_IV_TECH, false},
      {ECM, ECM_TECH, false},
      {BEAGLE_PROBE, BEAGLE_PROBE_TECH, false},
      {LIGHT_BAP, LIGHT_BAP_TECH, false},
      {ANGELECM, ANGEL_ECM_TECH, true},
      {TAG, TAG_TECH, true},
      {C3I, C3I_TECH, true},
      {BLOODHOUND_PROBE, BLOODHOUND_PROBE_TECH, true},
      {TARGETING_COMPUTER, TCOMP_TECH, true},
  };

  for (size_t index = 0; index < sizeof(EXPECTATIONS) / sizeof(*EXPECTATIONS);
       index++) {
    const SpecialTechnologyExpectation *expectation = checked_storage_at_const(
        EXPECTATIONS, sizeof(EXPECTATIONS) / sizeof(*EXPECTATIONS),
        sizeof(*EXPECTATIONS), index);
    reset_state();
    use_critical_grid = true;
    technology_flags = CLAN_TECH;
    secondary_technology_flags = STEALTH_ARMOR_TECH;

    btech_admin_special_install(mech, expectation->special, HEAD, 0, 0);
    const int INSTALLED_FLAGS =
        expectation->secondary ? secondary_technology_flags : technology_flags;
    assert((INSTALLED_FLAGS & expectation->flag) != 0);

    btech_admin_special_install(mech, -1, HEAD, 0, 0);
    const int CLEARED_FLAGS =
        expectation->secondary ? secondary_technology_flags : technology_flags;
    assert((CLEARED_FLAGS & expectation->flag) == 0);
    assert((technology_flags & CLAN_TECH) != 0);
    assert((secondary_technology_flags & STEALTH_ARMOR_TECH) != 0);
  }
}

static void test_special_replacement_reconciles_old_and_new_metadata(void) {
  reset_state();
  use_critical_grid = true;
  technology_flags = CLAN_TECH;
  secondary_technology_flags = STEALTH_ARMOR_TECH;

  btech_admin_special_install(mech, ECM, HEAD, 0, 0);
  assert((technology_flags & ECM_TECH) != 0);

  btech_admin_special_install(mech, TAG, HEAD, 0, 0);
  assert((technology_flags & ECM_TECH) == 0);
  assert((secondary_technology_flags & TAG_TECH) != 0);
  assert((technology_flags & CLAN_TECH) != 0);
  assert((secondary_technology_flags & STEALTH_ARMOR_TECH) != 0);
}

static void test_special_replacement_preserves_metadata_for_duplicate(void) {
  reset_state();
  use_critical_grid = true;

  btech_admin_special_install(mech, ECM, HEAD, 0, 0);
  btech_admin_special_install(mech, ECM, HEAD, 0, 0);
  assert((technology_flags & ECM_TECH) != 0);
  btech_admin_special_install(mech, ECM, CTORSO, 1, 0);
  btech_admin_special_install(mech, -1, HEAD, 0, 0);
  assert((technology_flags & ECM_TECH) != 0);

  btech_admin_special_install(mech, -1, CTORSO, 1, 0);
  assert((technology_flags & ECM_TECH) == 0);
}

static void test_case_reconciliation_tracks_configuration_destination(void) {
  reset_state();
  use_critical_grid = true;
  unit_class = CLASS_MECH;
  *section_configuration_address(HEAD) = SECTION_BREACHED;

  btech_admin_special_install(mech, CASE, HEAD, 0, 0);
  btech_admin_special_install(mech, CASE, HEAD, 1, 0);
  assert((*section_configuration_address(HEAD) & CASE_TECH) != 0);

  btech_admin_special_install(mech, -1, HEAD, 0, 0);
  assert((*section_configuration_address(HEAD) & CASE_TECH) != 0);
  assert(case_configuration_removals == 0);

  btech_admin_special_install(mech, -1, HEAD, 1, 0);
  assert((*section_configuration_address(HEAD) & CASE_TECH) == 0);
  assert((*section_configuration_address(HEAD) & SECTION_BREACHED) != 0);
  assert(case_configuration_removals == 1);

  reset_state();
  use_critical_grid = true;
  unit_class = CLASS_VEH_GROUND;
  btech_admin_special_install(mech, CASE, HEAD, 0, 0);
  btech_admin_special_install(mech, CASE, CTORSO, 0, 0);
  assert((*section_configuration_address(BSIDE) & CASE_TECH) != 0);

  btech_admin_special_install(mech, -1, HEAD, 0, 0);
  assert((*section_configuration_address(BSIDE) & CASE_TECH) != 0);
  btech_admin_special_install(mech, -1, CTORSO, 0, 0);
  assert((*section_configuration_address(BSIDE) & CASE_TECH) == 0);
}

static void test_removing_movement_technology_removes_related_criticals(void) {
  reset_state();
  use_critical_grid = true;
  technology_flags = TRIPLE_MYOMER_TECH | MASC_TECH | ECM_TECH;
  *critical_part_address(HEAD, 0) =
      special_equipment_index(TRIPLE_STRENGTH_MYOMER);
  *critical_part_address(CTORSO, 1) = special_equipment_index(MASC);

  btech_admin_technology_set(mech, BTECH_ADMIN_TECHNOLOGY_PRIMARY,
                             TRIPLE_MYOMER_TECH, false);
  assert((technology_flags & TRIPLE_MYOMER_TECH) == 0);
  assert((technology_flags & MASC_TECH) != 0);
  assert((technology_flags & ECM_TECH) != 0);
  assert(*critical_part_address(HEAD, 0) == EMPTY);
  assert(*critical_part_address(CTORSO, 1) == special_equipment_index(MASC));

  btech_admin_technology_set(mech, BTECH_ADMIN_TECHNOLOGY_PRIMARY, MASC_TECH,
                             false);
  assert((technology_flags & MASC_TECH) == 0);
  assert((technology_flags & ECM_TECH) != 0);
  assert(*critical_part_address(CTORSO, 1) == EMPTY);

  *critical_part_address(LARM, 2) =
      special_equipment_index(TRIPLE_STRENGTH_MYOMER);
  btech_admin_technology_set(mech, BTECH_ADMIN_TECHNOLOGY_PRIMARY,
                             TRIPLE_MYOMER_TECH, false);
  assert(*critical_part_address(LARM, 2) ==
         special_equipment_index(TRIPLE_STRENGTH_MYOMER));
}

static void test_removing_case_clears_criticals_and_section_flags(void) {
  reset_state();
  use_critical_grid = true;
  *critical_part_address(HEAD, 0) = special_equipment_index(CASE);
  *critical_part_address(CTORSO, 1) = special_equipment_index(CASE);
  *critical_part_address(LARM, 2) = special_equipment_index(MASC);

  btech_admin_case_remove(mech);

  assert(*critical_part_address(HEAD, 0) == EMPTY);
  assert(*critical_part_address(CTORSO, 1) == EMPTY);
  assert(*critical_part_address(LARM, 2) == special_equipment_index(MASC));
  assert(case_configuration_removals == NUM_SECTIONS);
}

static void invoke_setcargospace(const char *input) {
  char command[64];

  assert(strlen(input) < sizeof(command));
  assert(snprintf(command, sizeof(command), "%s", input) >= 0);
  mechrep_setcargospace(99, mech, command);
}

static void assert_cargo_unchanged(const char *input) {
  cargo_space = 456;
  carrier_maximum_tonnage = 78;
  invoke_setcargospace(input);
  assert(cargo_space == 456);
  assert(carrier_maximum_tonnage == 78);
}

static void test_setcargospace_is_atomic_and_exact(void) {
  reset_state();
  invoke_setcargospace("0 1");
  assert(cargo_space == 0);
  assert(carrier_maximum_tonnage == 1);

  reset_state();
  invoke_setcargospace("5000 100");
  assert(cargo_space == 250000);
  assert(carrier_maximum_tonnage == 100);

  reset_state();
  assert_cargo_unchanged("junk 20");
  assert_cargo_unchanged("20 junk");
  assert_cargo_unchanged("2147483648 20");
  assert_cargo_unchanged("20 2147483648");
  assert_cargo_unchanged("20 30 trailing");
  assert_cargo_unchanged("5001 20");
}

static void test_setcargospace_clamps_maximum_tonnage(void) {
  reset_state();
  invoke_setcargospace("1 -5");
  assert(cargo_space == 50);
  assert(carrier_maximum_tonnage == 1);

  reset_state();
  invoke_setcargospace("2 0");
  assert(cargo_space == 100);
  assert(carrier_maximum_tonnage == 1);

  reset_state();
  invoke_setcargospace("3 101");
  assert(cargo_space == 150);
  assert(carrier_maximum_tonnage == 100);
}

static void test_setcargospace_rejects_unavailable_contexts(void) {
  reset_state();
  command_status = REPAIR_COMMAND_UNAUTHORIZED;
  invoke_setcargospace("20 30");
  assert(cargo_space == 456);
  assert(carrier_maximum_tonnage == 78);

  reset_state();
  command_status = REPAIR_COMMAND_MISSING_MECH;
  invoke_setcargospace("20 30");
  assert(cargo_space == 456);
  assert(carrier_maximum_tonnage == 78);
}

static void invoke_restock(const char *input) {
  char command[64];

  assert(strlen(input) < sizeof(command));
  assert(snprintf(command, sizeof(command), "%s", input) >= 0);
  mechrep_rrestock(99, mech, command);
}

static void assert_restock_rejected_without_ammunition_lookups(int part_type) {
  reset_state();
  special_slot.part_type = part_type;
  special_slot.data = 19;

  invoke_restock("HEAD 1");

  assert(critical_part_type_calls == 1);
  assert(ammunition_per_ton_calls == 0);
  assert(full_ammo_calls == 0);
  assert(data_set_calls == 0);
  assert(special_slot.data == 19);
}

static void test_restock_rejects_non_ammunition_slots_without_lookup(void) {
  assert_restock_rejected_without_ammunition_lookups(EMPTY);
  assert_restock_rejected_without_ammunition_lookups(weapon_equipment_index(7));
  assert_restock_rejected_without_ammunition_lookups(
      special_equipment_index(7));
}

static void test_restock_refills_valid_ammunition(void) {
  reset_state();
  special_slot.part_type = ammunition_equipment_index(7);
  special_slot.data = 19;
  special_slot.fire_mode &= ~DESTROYED_MODE;
  const int FIRE_MODE = special_slot.fire_mode;
  const int AMMO_MODE = special_slot.ammo_mode;
  const int BRAND = special_slot.brand;

  invoke_restock("HEAD 1");

  assert(critical_part_type_calls == 1);
  assert(ammunition_per_ton_calls == 1);
  assert(full_ammo_calls == 1);
  assert(data_set_calls == 1);
  assert(special_slot.data == 42);
  assert(special_slot.fire_mode == FIRE_MODE);
  assert(special_slot.ammo_mode == AMMO_MODE);
  assert(special_slot.brand == BRAND);
}

static void test_restock_rejects_destroyed_ammunition_without_mutation(void) {
  reset_state();
  special_slot.part_type = ammunition_equipment_index(7);
  special_slot.data = 19;
  critical_destroyed = true;
  const int FIRE_MODE = special_slot.fire_mode;
  const int AMMO_MODE = special_slot.ammo_mode;
  const int BRAND = special_slot.brand;

  invoke_restock("HEAD 1");

  assert(critical_part_type_calls == 1);
  assert(ammunition_per_ton_calls == 0);
  assert(full_ammo_calls == 0);
  assert(data_set_calls == 0);
  assert(special_slot.data == 19);
  assert(special_slot.fire_mode == FIRE_MODE);
  assert(special_slot.ammo_mode == AMMO_MODE);
  assert(special_slot.brand == BRAND);
}

static void test_restock_rejects_invalid_or_unavailable_contexts(void) {
  reset_state();
  special_slot.part_type = ammunition_equipment_index(7);
  invoke_restock("HEAD 3");
  assert(critical_part_type_calls == 0);
  assert(ammunition_per_ton_calls == 0);
  assert(full_ammo_calls == 0);
  assert(data_set_calls == 0);

  reset_state();
  special_slot.part_type = ammunition_equipment_index(7);
  special_slot.data = 19;
  invoke_restock("HEAD -2147483648");
  assert(critical_part_type_calls == 0);
  assert(ammunition_per_ton_calls == 0);
  assert(full_ammo_calls == 0);
  assert(data_set_calls == 0);
  assert(special_slot.data == 19);

  reset_state();
  special_slot.part_type = ammunition_equipment_index(7);
  command_status = REPAIR_COMMAND_MISSING_MECH;
  invoke_restock("HEAD 1");
  assert(critical_part_type_calls == 0);
  assert(ammunition_per_ton_calls == 0);
  assert(full_ammo_calls == 0);
  assert(data_set_calls == 0);

  reset_state();
  special_slot.part_type = ammunition_equipment_index(7);
  command_status = REPAIR_COMMAND_UNAUTHORIZED;
  invoke_restock("HEAD 1");
  assert(critical_part_type_calls == 0);
  assert(ammunition_per_ton_calls == 0);
  assert(full_ammo_calls == 0);
  assert(data_set_calls == 0);
}

int main(void) {
  test_delinftech_uses_repair_target();
  test_addweap_validates_before_mutation();
  test_raddspecial_replaces_slot_with_fresh_state();
  test_special_install_reconciles_every_derived_technology();
  test_special_replacement_reconciles_old_and_new_metadata();
  test_special_replacement_preserves_metadata_for_duplicate();
  test_case_reconciliation_tracks_configuration_destination();
  test_removing_movement_technology_removes_related_criticals();
  test_removing_case_clears_criticals_and_section_flags();
  test_setcargospace_is_atomic_and_exact();
  test_setcargospace_clamps_maximum_tonnage();
  test_setcargospace_rejects_unavailable_contexts();
  test_restock_rejects_non_ammunition_slots_without_lookup();
  test_restock_refills_valid_ammunition();
  test_restock_rejects_destroyed_ammunition_without_mutation();
  test_restock_rejects_invalid_or_unavailable_contexts();
  return 0;
}
