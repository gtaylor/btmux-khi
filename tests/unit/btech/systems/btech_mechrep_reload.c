#include "btech/repair/mechrep_api.h"

#undef NDEBUG
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "btech/context.h"
#include "btech/repair/repair_job.h"
#include "btech/scripting/command_handlers_api.h"
#include "btech/special/registry_api.h"
#include "btech/unit/equipment_types.h"
#include "btech/unit/mech_build_api.h"
#include "btech/unit/mech_classification_api.h"
#include "btech/unit/mech_equipment_api.h"
#include "btech/unit/mech_identity_api.h"
#include "btech/unit/mech_specification_api.h"
#include "btech/unit/mech_status_types.h"
#include "btech/unit/mech_utils_api.h"
#include "btech/unit/section_types.h"
#include "btech/unit/template_api.h"
#include "btech/unit/weapon_catalogue_api.h"
#include "mux/network/network_output.h"
#include "mux/server/platform.h"
#include "mux/support/checked_storage.h"

enum {
  TEST_WEAPON = 7,
  CRITICAL_CAPACITY = NUM_SECTIONS * NUM_CRITICALS,
};

static BtechContext *const CONTEXT = (BtechContext *)1;
static EvaluationContext *const EVALUATION = (EvaluationContext *)2;
static Mech *const MECH = (Mech *)3;
static int parts[CRITICAL_CAPACITY];
static int data_values[CRITICAL_CAPACITY];
static int fire_modes[CRITICAL_CAPACITY];
static int ammunition_modes[CRITICAL_CAPACITY];
static int damage_flags[CRITICAL_CAPACITY];
static int raw_brands[CRITICAL_CAPACITY];
static int desired_ammunition_sections[CRITICAL_CAPACITY];
static int technology_flags;
static int configure_count;

static int *slot_value(int *values, int section, int critical) {
  return checked_storage_at(values, CRITICAL_CAPACITY, sizeof(*values),
                            ((size_t)section * (size_t)NUM_CRITICALS) +
                                (size_t)critical);
}

static void reset_state(void) {
  memset(parts, 0, sizeof(parts));
  memset(data_values, 0, sizeof(data_values));
  memset(fire_modes, 0, sizeof(fire_modes));
  memset(ammunition_modes, 0, sizeof(ammunition_modes));
  memset(damage_flags, 0, sizeof(damage_flags));
  memset(raw_brands, 0, sizeof(raw_brands));
  for (size_t index = 0; index < CRITICAL_CAPACITY; index++)
    *(int *)checked_storage_at(desired_ammunition_sections, CRITICAL_CAPACITY,
                               sizeof(*desired_ammunition_sections), index) =
        -1;
  technology_flags = 0;
  configure_count = 0;
}

RepairCommandStatus
mech_admin_command_context_initialize(DbRef player, void *data,
                                      MechAdminCommandContext *command) {
  assert(data == MECH);
  *command = (MechAdminCommandContext){.player = player,
                                       .context = CONTEXT,
                                       .evaluation = EVALUATION,
                                       .mech = MECH};
  return REPAIR_COMMAND_READY;
}

const char *repair_command_status_message(RepairCommandStatus status
                                          [[maybe_unused]]) {
  return "unavailable";
}

EvaluationContext *btech_context_evaluation(BtechContext *context) {
  return context == CONTEXT ? EVALUATION : nullptr;
}

BtechContext *mech_context(const Mech *mech [[maybe_unused]]) {
  return CONTEXT;
}

int mech_parseattributes(char *buffer, char **args, int maxargs) {
  int count = 0;
  for (char *token = strtok(buffer, " "); token && count < maxargs;
       token = strtok(nullptr, " "))
    *(char **)checked_storage_at((void *)args, (size_t)maxargs, sizeof(*args),
                                 (size_t)count++) = token;
  return count;
}

int weapon_index_from_string(BtechContext *context, const char *string) {
  return context == CONTEXT && strcmp(string, "Laser") == 0 ? TEST_WEAPON : -1;
}

UnitClass mech_class(const Mech *mech [[maybe_unused]]) { return CLASS_MECH; }

MechMovementType mech_movement_type(const Mech *mech [[maybe_unused]]) {
  return MOVE_BIPED;
}

int armor_section_from_string(UnitClass type [[maybe_unused]],
                              MechMovementType movement_type [[maybe_unused]],
                              const char *string) {
  return strcmp(string, "HEAD") == 0 ? HEAD : -1;
}

int crits_in_loc(Mech *mech [[maybe_unused]], int index [[maybe_unused]]) {
  return NUM_CRITICALS;
}

int weapon_catalogue_ammunition_per_ton(int weapon) {
  return weapon == TEST_WEAPON ? 20 : 0;
}

int mech_section_critical_count(Mech *mech [[maybe_unused]],
                                int section [[maybe_unused]]) {
  return NUM_CRITICALS;
}

int mech_critical_part_type(const Mech *mech [[maybe_unused]], int section,
                            int critical) {
  return *slot_value(parts, section, critical);
}

int mech_critical_fire_mode(const Mech *mech [[maybe_unused]], int section,
                            int critical) {
  return *slot_value(fire_modes, section, critical);
}

int mech_critical_ammo_mode(const Mech *mech [[maybe_unused]], int section,
                            int critical) {
  return *slot_value(ammunition_modes, section, critical);
}

void mech_critical_configure(const CriticalSlotConfiguration *configuration) {
  const int SECTION = configuration->slot.section;
  const int CRITICAL = configuration->slot.critical;
  configure_count++;
  *slot_value(parts, SECTION, CRITICAL) = configuration->part_type;
  *slot_value(data_values, SECTION, CRITICAL) = configuration->data;
  *slot_value(fire_modes, SECTION, CRITICAL) = configuration->fire_mode;
  *slot_value(ammunition_modes, SECTION, CRITICAL) = configuration->ammo_mode;
}

void mech_critical_damage_flags_set(Mech *mech [[maybe_unused]], int section,
                                    int critical, int flags) {
  *slot_value(damage_flags, section, critical) = flags;
}

void mech_critical_temporary_failure_set(
    const CriticalSlotFailureSet *request) {
  int *raw_brand =
      slot_value(raw_brands, request->slot.section, request->slot.critical);
  *raw_brand = (*raw_brand & 0x0F) | (request->failure << 4);
}

void mech_critical_brand_set(const CriticalSlotBrandSet *request) {
  int *raw_brand =
      slot_value(raw_brands, request->slot.section, request->slot.critical);
  *raw_brand = (*raw_brand & 0xF0) | request->brand;
}

void mech_critical_desired_ammo_section_set(Mech *mech [[maybe_unused]],
                                            int section, int critical,
                                            int ammo_section) {
  *slot_value(desired_ammunition_sections, section, critical) = ammo_section;
}

void mech_critical_data_set(Mech *mech [[maybe_unused]], int section,
                            int critical, int data) {
  *slot_value(data_values, section, critical) = data;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): Stubbed public API.
int full_ammo(const Mech *mech [[maybe_unused]], int loc [[maybe_unused]],
              int pos [[maybe_unused]]) {
  return 42;
}

void mech_technology_flags_remove(Mech *mech [[maybe_unused]], int flags) {
  technology_flags &= ~flags;
}

void mech_technology_flags_secondary_remove(Mech *mech [[maybe_unused]],
                                            int flags [[maybe_unused]]) {}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Stubbed public API.
void mech_section_configuration_remove(Mech *mech [[maybe_unused]],
                                       int section [[maybe_unused]],
                                       int configuration [[maybe_unused]]) {}
// NOLINTEND(bugprone-easily-swappable-parameters)

bool weapon_catalogue_is_anti_missile(int weapon_index [[maybe_unused]]) {
  return false;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): Stubbed public API.
bool weapon_catalogue_has_special(int weapon_index [[maybe_unused]],
                                  int special [[maybe_unused]]) {
  return false;
}

void mecha_notify(EvaluationContext *evaluation [[maybe_unused]],
                  DbRef player [[maybe_unused]],
                  const char *msg [[maybe_unused]]) {}

void notify_printf(EvaluationContext *evaluation [[maybe_unused]],
                   DbRef player [[maybe_unused]],
                   const char *format [[maybe_unused]], ...) {}

void dump_weapons(BtechContext *context [[maybe_unused]],
                  DbRef player [[maybe_unused]]) {}

static void invoke_reload(const char *input) {
  char command[64];
  assert(snprintf(command, sizeof(command), "%s", input) >= 0);
  mechrep_rreload(99, MECH, command);
}

static void seed_stale_slot(int section, int critical) {
  *slot_value(parts, section, critical) = special_equipment_index(ECM);
  *slot_value(data_values, section, critical) = 19;
  *slot_value(fire_modes, section, critical) = DESTROYED_MODE | DAMAGED_MODE;
  *slot_value(ammunition_modes, section, critical) = ARTEMIS_MODE;
  *slot_value(damage_flags, section, critical) = WEAP_DAM_MODERATE;
  *slot_value(raw_brands, section, critical) = 0xF9;
  *slot_value(desired_ammunition_sections, section, critical) = RTORSO;
}

static void test_reload_replaces_once_with_fresh_metadata(void) {
  reset_state();
  seed_stale_slot(HEAD, 0);
  technology_flags = ECM_TECH;

  invoke_reload("Laser HEAD 1 I");

  assert(configure_count == 1);
  assert(*slot_value(parts, HEAD, 0) ==
         ammunition_equipment_index(TEST_WEAPON));
  assert(*slot_value(data_values, HEAD, 0) == 42);
  assert(*slot_value(fire_modes, HEAD, 0) == 0);
  assert(*slot_value(ammunition_modes, HEAD, 0) == INFERNO_MODE);
  assert(*slot_value(damage_flags, HEAD, 0) == 0);
  assert(*slot_value(raw_brands, HEAD, 0) == 0);
  assert(*slot_value(desired_ammunition_sections, HEAD, 0) == -1);
  assert((technology_flags & ECM_TECH) == 0);
}

static void test_reload_preserves_duplicate_provider_and_half_ton_modes(void) {
  reset_state();
  seed_stale_slot(HEAD, 0);
  *slot_value(parts, CTORSO, 0) = special_equipment_index(ECM);
  *slot_value(fire_modes, HEAD, 0) =
      HALFTON_MODE | WILL_JETTISON_MODE | DESTROYED_MODE;
  *slot_value(ammunition_modes, HEAD, 0) = ARTEMIS_MODE | 0x40000000;
  technology_flags = ECM_TECH;

  invoke_reload("Laser HEAD 1 W");

  assert(configure_count == 1);
  assert(*slot_value(fire_modes, HEAD, 0) == HALFTON_MODE);
  assert(*slot_value(ammunition_modes, HEAD, 0) == (ARTEMIS_MODE | SWARM_MODE));
  assert((technology_flags & ECM_TECH) != 0);
}

static void test_reload_dash_clears_modes(void) {
  reset_state();
  *slot_value(fire_modes, HEAD, 0) = HALFTON_MODE | WILL_JETTISON_MODE;
  *slot_value(ammunition_modes, HEAD, 0) = ARTEMIS_MODE;

  invoke_reload("Laser HEAD 1 -");

  assert(configure_count == 1);
  assert(*slot_value(fire_modes, HEAD, 0) == 0);
  assert(*slot_value(ammunition_modes, HEAD, 0) == 0);
}

static void test_reload_rejects_invalid_slot_without_mutation(void) {
  reset_state();
  seed_stale_slot(HEAD, 0);

  invoke_reload("Laser HEAD 13 I");

  assert(configure_count == 0);
  assert(*slot_value(parts, HEAD, 0) == special_equipment_index(ECM));
  assert(*slot_value(data_values, HEAD, 0) == 19);
  assert(*slot_value(raw_brands, HEAD, 0) == 0xF9);
}

int main(void) {
  test_reload_replaces_once_with_fresh_metadata();
  test_reload_preserves_duplicate_provider_and_half_ton_modes();
  test_reload_dash_clears_modes();
  test_reload_rejects_invalid_slot_without_mutation();
  return 0;
}
