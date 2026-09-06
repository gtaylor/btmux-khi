#include "btech/repair/mechrep_api.h"

#undef NDEBUG
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "btech/unit/equipment_types.h"
#include "btech/unit/mech_build_api.h"
#include "btech/unit/mech_classification_api.h"
#include "btech/unit/mech_equipment_api.h"
#include "btech/unit/mech_specification_api.h"
#include "btech/unit/mech_status_types.h"
#include "btech/unit/mech_utils_api.h"
#include "btech/unit/section_types.h"
#include "btech/unit/weapon_catalogue_api.h"
#include "mux/support/checked_storage.h"

enum {
  TEST_WEAPON = 7,
  IS_AMS_WEAPON = 8,
  CLAN_AMS_WEAPON = 9,
  CRITICAL_PART_CAPACITY = NUM_SECTIONS * NUM_CRITICALS,
};

static Mech *const MECH = (Mech *)1;
static int critical_parts[CRITICAL_PART_CAPACITY];
static int default_critical_parts[CRITICAL_PART_CAPACITY];
static int critical_brands[CRITICAL_PART_CAPACITY];
static int critical_data_values[CRITICAL_PART_CAPACITY];
static int critical_fire_modes[CRITICAL_PART_CAPACITY];
static int critical_ammo_modes[CRITICAL_PART_CAPACITY];
static int critical_damage_flags[CRITICAL_PART_CAPACITY];
static int critical_desired_ammo_sections[CRITICAL_PART_CAPACITY];
static int section_configurations[NUM_SECTIONS];
static int technology_flags;
static int secondary_technology_flags;
static UnitClass unit_class;
static bool weapon_is_ams;
static bool weapon_is_clan;

static int *critical_part_address(int section, int critical) {
  return checked_storage_at(
      critical_parts, CRITICAL_PART_CAPACITY, sizeof(*critical_parts),
      ((size_t)section * (size_t)NUM_CRITICALS) + (size_t)critical);
}

static int *critical_metadata_address(int *metadata, int section,
                                      int critical) {
  return checked_storage_at(metadata, CRITICAL_PART_CAPACITY, sizeof(*metadata),
                            ((size_t)section * (size_t)NUM_CRITICALS) +
                                (size_t)critical);
}

static int *section_configuration_address(int section) {
  return checked_storage_at(section_configurations, NUM_SECTIONS,
                            sizeof(*section_configurations), (size_t)section);
}

static void reset_state(void) {
  for (size_t index = 0; index < CRITICAL_PART_CAPACITY; index++) {
    *(int *)checked_storage_at(critical_parts, CRITICAL_PART_CAPACITY,
                               sizeof(*critical_parts), index) = EMPTY;
    *(int *)checked_storage_at(default_critical_parts, CRITICAL_PART_CAPACITY,
                               sizeof(*default_critical_parts), index) = EMPTY;
  }
  for (size_t index = 0; index < CRITICAL_PART_CAPACITY; index++) {
    *(int *)checked_storage_at(critical_brands, CRITICAL_PART_CAPACITY,
                               sizeof(*critical_brands), index) = 0;
    *(int *)checked_storage_at(critical_data_values, CRITICAL_PART_CAPACITY,
                               sizeof(*critical_data_values), index) = 0;
    *(int *)checked_storage_at(critical_fire_modes, CRITICAL_PART_CAPACITY,
                               sizeof(*critical_fire_modes), index) = 0;
    *(int *)checked_storage_at(critical_ammo_modes, CRITICAL_PART_CAPACITY,
                               sizeof(*critical_ammo_modes), index) = 0;
    *(int *)checked_storage_at(critical_damage_flags, CRITICAL_PART_CAPACITY,
                               sizeof(*critical_damage_flags), index) = 0;
    *(int *)checked_storage_at(
        critical_desired_ammo_sections, CRITICAL_PART_CAPACITY,
        sizeof(*critical_desired_ammo_sections), index) = -1;
  }
  for (size_t index = 0; index < NUM_SECTIONS; index++)
    *(int *)checked_storage_at(section_configurations, NUM_SECTIONS,
                               sizeof(*section_configurations), index) = 0;
  technology_flags = 0;
  secondary_technology_flags = 0;
  unit_class = CLASS_MECH;
  weapon_is_ams = false;
  weapon_is_clan = false;
}

void fill_default_criticals(Mech *mech [[maybe_unused]], int index) {
  for (int slot = 0; slot < NUM_CRITICALS; slot++) {
    *critical_part_address(index, slot) =
        *critical_metadata_address(default_critical_parts, index, slot);
    *critical_metadata_address(critical_data_values, index, slot) = 40 + slot;
    *critical_metadata_address(critical_fire_modes, index, slot) = REAR_MOUNT;
    *critical_metadata_address(critical_ammo_modes, index, slot) = ARTEMIS_MODE;
  }
}

UnitClass mech_class(const Mech *mech [[maybe_unused]]) { return unit_class; }

int mech_section_critical_count(Mech *mech [[maybe_unused]],
                                int section [[maybe_unused]]) {
  return NUM_CRITICALS;
}

int mech_critical_part_type(const Mech *mech [[maybe_unused]], int section,
                            int critical) {
  return *critical_part_address(section, critical);
}

void mech_critical_configure(const CriticalSlotConfiguration *configuration) {
  const int SECTION = configuration->slot.section;
  const int CRITICAL = configuration->slot.critical;
  *critical_part_address(SECTION, CRITICAL) = configuration->part_type;
  *critical_metadata_address(critical_data_values, SECTION, CRITICAL) =
      configuration->data;
  *critical_metadata_address(critical_fire_modes, SECTION, CRITICAL) =
      configuration->fire_mode;
  *critical_metadata_address(critical_ammo_modes, SECTION, CRITICAL) =
      configuration->ammo_mode;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Stubbed public API shapes.
void mech_critical_damage_flags_set(Mech *mech [[maybe_unused]], int section,
                                    int critical, int flags) {
  *critical_metadata_address(critical_damage_flags, section, critical) = flags;
}

void mech_critical_temporary_failure_set(
    const CriticalSlotFailureSet *request) {
  int *brand = critical_metadata_address(critical_brands, request->slot.section,
                                         request->slot.critical);
  *brand = (*brand & 0x0F) | (request->failure << 4);
}

void mech_critical_brand_set(const CriticalSlotBrandSet *request) {
  int *brand = critical_metadata_address(critical_brands, request->slot.section,
                                         request->slot.critical);
  *brand = (*brand & 0xF0) | request->brand;
}

void mech_critical_desired_ammo_section_set(Mech *mech [[maybe_unused]],
                                            int section, int critical,
                                            int ammo_section) {
  *critical_metadata_address(critical_desired_ammo_sections, section,
                             critical) = ammo_section;
}

int mech_critical_data(const Mech *mech [[maybe_unused]], int section,
                       int critical) {
  return *critical_metadata_address(critical_data_values, section, critical);
}

void mech_critical_data_set(Mech *mech [[maybe_unused]], int section,
                            int critical, int data) {
  *critical_metadata_address(critical_data_values, section, critical) = data;
}

int full_ammo(const Mech *mech [[maybe_unused]], int loc [[maybe_unused]],
              int pos [[maybe_unused]]) {
  return 42;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

void mech_section_configuration_remove(Mech *mech [[maybe_unused]], int section,
                                       int configuration) {
  *section_configuration_address(section) &= ~configuration;
}

void mech_section_configuration_add(Mech *mech [[maybe_unused]], int section,
                                    int configuration) {
  *section_configuration_address(section) |= configuration;
}

void mech_technology_flags_add(Mech *mech [[maybe_unused]], int flags) {
  technology_flags |= flags;
}

void mech_technology_flags_remove(Mech *mech [[maybe_unused]], int flags) {
  technology_flags &= ~flags;
}

int mech_technology_flags(const Mech *mech [[maybe_unused]]) {
  return technology_flags;
}

void mech_technology_flags_set(Mech *mech [[maybe_unused]], int flags) {
  technology_flags = flags;
}

void mech_technology_flags_secondary_remove(Mech *mech [[maybe_unused]],
                                            int flags) {
  secondary_technology_flags &= ~flags;
}

void mech_technology_flags_secondary_add(Mech *mech [[maybe_unused]],
                                         int flags) {
  secondary_technology_flags |= flags;
}

void mech_technology_flags_secondary_set(Mech *mech [[maybe_unused]],
                                         int flags) {
  secondary_technology_flags = flags;
}

void mech_infantry_technology_flags_set(Mech *mech [[maybe_unused]],
                                        int flags [[maybe_unused]]) {}

void mech_infantry_technology_flags_add(Mech *mech [[maybe_unused]],
                                        int flags [[maybe_unused]]) {}

int mech_infantry_technology_flags(const Mech *mech [[maybe_unused]]) {
  return 0;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): Catalogue API shape.
bool weapon_catalogue_has_special(int weapon, int special) {
  if (special == AMS)
    return weapon == IS_AMS_WEAPON || weapon == CLAN_AMS_WEAPON ||
           (weapon == TEST_WEAPON && weapon_is_ams);
  if (special == CLAT)
    return weapon == CLAN_AMS_WEAPON ||
           (weapon == TEST_WEAPON && weapon_is_clan);
  return false;
}

bool weapon_catalogue_is_anti_missile(int weapon) {
  return weapon == IS_AMS_WEAPON || weapon == CLAN_AMS_WEAPON ||
         (weapon == TEST_WEAPON && weapon_is_ams);
}

typedef struct SpecialTechnologyExpectation {
  int special;
  int flag;
  bool secondary;
} SpecialTechnologyExpectation;

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

static void install_weapon(int section, const int *slots, size_t slot_count) {
  btech_admin_weapon_install(MECH, TEST_WEAPON, slots, slot_count, section, 0);
  for (size_t index = 0; index < slot_count; index++) {
    const int SLOT = *(const int *)checked_storage_at_const(
        slots, slot_count, sizeof(*slots), index);
    assert(*critical_part_address(section, SLOT) ==
           weapon_equipment_index(TEST_WEAPON));
  }
}

static void test_reconciles_every_special_technology(void) {
  const int SLOT[] = {0};

  for (size_t index = 0; index < sizeof(EXPECTATIONS) / sizeof(*EXPECTATIONS);
       index++) {
    const SpecialTechnologyExpectation *expectation = checked_storage_at_const(
        EXPECTATIONS, sizeof(EXPECTATIONS) / sizeof(*EXPECTATIONS),
        sizeof(*EXPECTATIONS), index);
    reset_state();
    technology_flags = CLAN_TECH;
    secondary_technology_flags = STEALTH_ARMOR_TECH;
    if (expectation->secondary)
      secondary_technology_flags |= expectation->flag;
    else
      technology_flags |= expectation->flag;
    *critical_part_address(HEAD, 0) =
        special_equipment_index(expectation->special);

    install_weapon(HEAD, SLOT, 1);

    const int RESULT =
        expectation->secondary ? secondary_technology_flags : technology_flags;
    assert((RESULT & expectation->flag) == 0);
    assert((technology_flags & CLAN_TECH) != 0);
    assert((secondary_technology_flags & STEALTH_ARMOR_TECH) != 0);
  }
}

static void test_duplicate_and_multi_slot_providers(void) {
  const int FIRST_SLOT[] = {0};
  const int BOTH_SLOTS[] = {0, 1};

  reset_state();
  technology_flags = ECM_TECH;
  *critical_part_address(HEAD, 0) = special_equipment_index(ECM);
  *critical_part_address(CTORSO, 0) = special_equipment_index(ECM);
  install_weapon(HEAD, FIRST_SLOT, 1);
  assert((technology_flags & ECM_TECH) != 0);
  install_weapon(CTORSO, FIRST_SLOT, 1);
  assert((technology_flags & ECM_TECH) == 0);

  reset_state();
  technology_flags = ECM_TECH;
  secondary_technology_flags = TAG_TECH;
  *critical_part_address(HEAD, 0) = special_equipment_index(ECM);
  *critical_part_address(HEAD, 1) = special_equipment_index(TAG);
  install_weapon(HEAD, BOTH_SLOTS, 2);
  assert((technology_flags & ECM_TECH) == 0);
  assert((secondary_technology_flags & TAG_TECH) == 0);

  reset_state();
  technology_flags = ECM_TECH;
  *critical_part_address(HEAD, 0) = special_equipment_index(ECM);
  *critical_part_address(HEAD, 1) = special_equipment_index(ECM);
  install_weapon(HEAD, BOTH_SLOTS, 2);
  assert((technology_flags & ECM_TECH) == 0);
}

static void test_case_reconciliation(void) {
  const int SLOT[] = {0};

  reset_state();
  *section_configuration_address(HEAD) = CASE_TECH | SECTION_BREACHED;
  *critical_part_address(HEAD, 0) = special_equipment_index(CASE);
  install_weapon(HEAD, SLOT, 1);
  assert((*section_configuration_address(HEAD) & CASE_TECH) == 0);
  assert((*section_configuration_address(HEAD) & SECTION_BREACHED) != 0);

  reset_state();
  unit_class = CLASS_VEH_GROUND;
  *section_configuration_address(BSIDE) = CASE_TECH | SECTION_BREACHED;
  *critical_part_address(HEAD, 0) = special_equipment_index(CASE);
  *critical_part_address(CTORSO, 0) = special_equipment_index(CASE);
  install_weapon(HEAD, SLOT, 1);
  assert((*section_configuration_address(BSIDE) & CASE_TECH) != 0);
  install_weapon(CTORSO, SLOT, 1);
  assert((*section_configuration_address(BSIDE) & CASE_TECH) == 0);
  assert((*section_configuration_address(BSIDE) & SECTION_BREACHED) != 0);
}

static void test_ams_and_unrelated_metadata_are_preserved(void) {
  const int SLOT[] = {0};

  reset_state();
  weapon_is_ams = true;
  technology_flags = MASC_TECH | CLAN_TECH;
  secondary_technology_flags = STEALTH_ARMOR_TECH;
  *critical_part_address(HEAD, 0) = special_equipment_index(MASC);
  install_weapon(HEAD, SLOT, 1);
  assert((technology_flags & MASC_TECH) == 0);
  assert((technology_flags & IS_ANTI_MISSILE_TECH) != 0);
  assert((technology_flags & CLAN_TECH) != 0);
  assert((secondary_technology_flags & STEALTH_ARMOR_TECH) != 0);

  reset_state();
  weapon_is_ams = true;
  weapon_is_clan = true;
  *critical_part_address(HEAD, 0) = special_equipment_index(TAG);
  secondary_technology_flags = TAG_TECH;
  install_weapon(HEAD, SLOT, 1);
  assert((secondary_technology_flags & TAG_TECH) == 0);
  assert((technology_flags & CL_ANTI_MISSILE_TECH) != 0);

  reset_state();
  technology_flags = ECM_TECH;
  install_weapon(HEAD, SLOT, 1);
  assert((technology_flags & ECM_TECH) != 0);
}

typedef enum ReplacementWriter : int {
  REPLACEMENT_WRITER_WEAPON,
  REPLACEMENT_WRITER_SPECIAL,
  REPLACEMENT_WRITER_AMMUNITION,
} ReplacementWriter;

static void replace_slot(ReplacementWriter writer) {
  const int SLOT[] = {0};

  switch (writer) {
  case REPLACEMENT_WRITER_WEAPON:
    install_weapon(HEAD, SLOT, 1);
    break;
  case REPLACEMENT_WRITER_SPECIAL:
    btech_admin_special_install(MECH, ECM, HEAD, 0, 0);
    assert(*critical_part_address(HEAD, 0) == special_equipment_index(ECM));
    assert((technology_flags & ECM_TECH) != 0);
    break;
  case REPLACEMENT_WRITER_AMMUNITION:
    btech_admin_ammunition_configure(MECH, TEST_WEAPON, HEAD, 0, 0, 0);
    assert(*critical_part_address(HEAD, 0) ==
           ammunition_equipment_index(TEST_WEAPON));
    assert(*critical_metadata_address(critical_data_values, HEAD, 0) == 42);
    break;
  }
}

static void seed_slot_metadata(int section, int slot, int seed) {
  *critical_metadata_address(critical_brands, section, slot) =
      ((seed & 0x0F) << 4) | ((seed + 1) & 0x0F);
  *critical_metadata_address(critical_data_values, section, slot) = seed + 20;
  *critical_metadata_address(critical_fire_modes, section, slot) =
      DESTROYED_MODE | DAMAGED_MODE | seed;
  *critical_metadata_address(critical_ammo_modes, section, slot) =
      0x1000 | seed;
  *critical_metadata_address(critical_damage_flags, section, slot) =
      0x2000 | seed;
  *critical_metadata_address(critical_desired_ammo_sections, section, slot) =
      seed;
}

static void test_weapon_install_resets_each_selected_slot_metadata(void) {
  const int SLOTS[] = {2, 4};
  const int REQUESTED_FIRE_MODES = REAR_MOUNT | ON_TC | OS_MODE;

  reset_state();
  seed_slot_metadata(HEAD, 2, 3);
  seed_slot_metadata(HEAD, 3, 5);
  seed_slot_metadata(HEAD, 4, 7);

  btech_admin_weapon_install(MECH, TEST_WEAPON, SLOTS, 2, HEAD,
                             REQUESTED_FIRE_MODES);

  for (size_t index = 0; index < 2; index++) {
    const int SLOT =
        *(const int *)checked_storage_at_const(SLOTS, 2, sizeof(*SLOTS), index);
    assert(*critical_part_address(HEAD, SLOT) ==
           weapon_equipment_index(TEST_WEAPON));
    assert(*critical_metadata_address(critical_data_values, HEAD, SLOT) == 0);
    assert(*critical_metadata_address(critical_fire_modes, HEAD, SLOT) ==
           REQUESTED_FIRE_MODES);
    assert(*critical_metadata_address(critical_ammo_modes, HEAD, SLOT) == 0);
    assert(*critical_metadata_address(critical_damage_flags, HEAD, SLOT) == 0);
    assert(*critical_metadata_address(critical_brands, HEAD, SLOT) == 0);
    assert(*critical_metadata_address(critical_desired_ammo_sections, HEAD,
                                      SLOT) == -1);
  }

  assert(*critical_part_address(HEAD, 3) == EMPTY);
  assert(*critical_metadata_address(critical_brands, HEAD, 3) == 0x56);
  assert(*critical_metadata_address(critical_data_values, HEAD, 3) == 25);
  assert(*critical_metadata_address(critical_fire_modes, HEAD, 3) ==
         (DESTROYED_MODE | DAMAGED_MODE | 5));
  assert(*critical_metadata_address(critical_ammo_modes, HEAD, 3) == 0x1005);
  assert(*critical_metadata_address(critical_damage_flags, HEAD, 3) == 0x2005);
  assert(*critical_metadata_address(critical_desired_ammo_sections, HEAD, 3) ==
         5);
}

static void
test_ammunition_replacement_resets_slot_and_reconciles_provider(void) {
  const int REQUESTED_FIRE_MODES = HALFTON_MODE;
  const int REQUESTED_AMMUNITION_MODES = ARTEMIS_MODE | INFERNO_MODE;

  reset_state();
  technology_flags = ECM_TECH;
  *critical_part_address(HEAD, 0) = special_equipment_index(ECM);
  *critical_part_address(CTORSO, 0) = special_equipment_index(ECM);
  seed_slot_metadata(HEAD, 0, 3);

  btech_admin_ammunition_configure(MECH, TEST_WEAPON, HEAD, 0,
                                   REQUESTED_FIRE_MODES,
                                   REQUESTED_AMMUNITION_MODES);

  assert(*critical_part_address(HEAD, 0) ==
         ammunition_equipment_index(TEST_WEAPON));
  assert(*critical_metadata_address(critical_data_values, HEAD, 0) == 42);
  assert(*critical_metadata_address(critical_fire_modes, HEAD, 0) ==
         REQUESTED_FIRE_MODES);
  assert(*critical_metadata_address(critical_ammo_modes, HEAD, 0) ==
         REQUESTED_AMMUNITION_MODES);
  assert(*critical_metadata_address(critical_damage_flags, HEAD, 0) == 0);
  assert(*critical_metadata_address(critical_brands, HEAD, 0) == 0);
  assert(*critical_metadata_address(critical_desired_ammo_sections, HEAD, 0) ==
         -1);
  assert((technology_flags & ECM_TECH) != 0);

  btech_admin_ammunition_configure(MECH, TEST_WEAPON, CTORSO, 0, 0, 0);
  assert((technology_flags & ECM_TECH) == 0);
}

static void assert_slot_is_canonical_empty(int section, int slot) {
  assert(*critical_part_address(section, slot) == EMPTY);
  assert(*critical_metadata_address(critical_data_values, section, slot) == 0);
  assert(*critical_metadata_address(critical_fire_modes, section, slot) == 0);
  assert(*critical_metadata_address(critical_ammo_modes, section, slot) == 0);
  assert(*critical_metadata_address(critical_damage_flags, section, slot) == 0);
  assert(*critical_metadata_address(critical_brands, section, slot) == 0);
  assert(*critical_metadata_address(critical_desired_ammo_sections, section,
                                    slot) == -1);
}

static void assert_seeded_slot_is_unchanged(int section, int slot, int seed) {
  assert(*critical_metadata_address(critical_brands, section, slot) ==
         (((seed & 0x0F) << 4) | ((seed + 1) & 0x0F)));
  assert(*critical_metadata_address(critical_data_values, section, slot) ==
         seed + 20);
  assert(*critical_metadata_address(critical_fire_modes, section, slot) ==
         (DESTROYED_MODE | DAMAGED_MODE | seed));
  assert(*critical_metadata_address(critical_ammo_modes, section, slot) ==
         (0x1000 | seed));
  assert(*critical_metadata_address(critical_damage_flags, section, slot) ==
         (0x2000 | seed));
  assert(*critical_metadata_address(critical_desired_ammo_sections, section,
                                    slot) == seed);
}

static void test_case_remove_leaves_canonical_empty_slots(void) {
  reset_state();
  *critical_part_address(HEAD, 0) = special_equipment_index(CASE);
  *critical_part_address(CTORSO, 2) = special_equipment_index(CASE);
  *critical_part_address(HEAD, 1) = special_equipment_index(ECM);
  seed_slot_metadata(HEAD, 0, 3);
  seed_slot_metadata(CTORSO, 2, 5);
  seed_slot_metadata(HEAD, 1, 7);
  *section_configuration_address(HEAD) = CASE_TECH | SECTION_BREACHED;
  *section_configuration_address(CTORSO) = CASE_TECH;

  btech_admin_case_remove(MECH);

  assert_slot_is_canonical_empty(HEAD, 0);
  assert_slot_is_canonical_empty(CTORSO, 2);
  assert(*critical_part_address(HEAD, 1) == special_equipment_index(ECM));
  assert_seeded_slot_is_unchanged(HEAD, 1, 7);
  assert(*section_configuration_address(HEAD) == SECTION_BREACHED);
  assert(*section_configuration_address(CTORSO) == 0);
}

static void test_technology_remove_leaves_canonical_empty_slots(void) {
  reset_state();
  technology_flags = MASC_TECH | ECM_TECH;
  secondary_technology_flags = TAG_TECH;
  *critical_part_address(HEAD, 0) = special_equipment_index(MASC);
  *critical_part_address(CTORSO, 2) = special_equipment_index(MASC);
  *critical_part_address(HEAD, 1) = special_equipment_index(ECM);
  seed_slot_metadata(HEAD, 0, 3);
  seed_slot_metadata(CTORSO, 2, 5);
  seed_slot_metadata(HEAD, 1, 7);

  btech_admin_technology_set(MECH, BTECH_ADMIN_TECHNOLOGY_PRIMARY, MASC_TECH,
                             false);

  assert_slot_is_canonical_empty(HEAD, 0);
  assert_slot_is_canonical_empty(CTORSO, 2);
  assert(*critical_part_address(HEAD, 1) == special_equipment_index(ECM));
  assert_seeded_slot_is_unchanged(HEAD, 1, 7);
  assert(technology_flags == ECM_TECH);
  assert(secondary_technology_flags == TAG_TECH);
}

static void test_primary_technology_clear_canonicalizes_related_slots(void) {
  reset_state();
  technology_flags = TRIPLE_MYOMER_TECH | MASC_TECH | ECM_TECH;
  secondary_technology_flags = TAG_TECH;
  *critical_part_address(HEAD, 0) = special_equipment_index(CASE);
  *critical_part_address(CTORSO, 1) =
      special_equipment_index(TRIPLE_STRENGTH_MYOMER);
  *critical_part_address(LTORSO, 2) = special_equipment_index(MASC);
  *critical_part_address(HEAD, 3) = special_equipment_index(ECM);
  seed_slot_metadata(HEAD, 0, 2);
  seed_slot_metadata(CTORSO, 1, 4);
  seed_slot_metadata(LTORSO, 2, 6);
  seed_slot_metadata(HEAD, 3, 8);
  *section_configuration_address(HEAD) = CASE_TECH | SECTION_BREACHED;

  btech_admin_technologies_clear(MECH, BTECH_ADMIN_TECHNOLOGY_PRIMARY);

  assert_slot_is_canonical_empty(HEAD, 0);
  assert_slot_is_canonical_empty(CTORSO, 1);
  assert_slot_is_canonical_empty(LTORSO, 2);
  assert(*critical_part_address(HEAD, 3) == special_equipment_index(ECM));
  assert_seeded_slot_is_unchanged(HEAD, 3, 8);
  assert(*section_configuration_address(HEAD) == SECTION_BREACHED);
  assert(technology_flags == 0);
  assert(secondary_technology_flags == TAG_TECH);
}

static void test_removes_each_ams_family_through_every_writer(void) {
  static const int AMS_WEAPONS[] = {IS_AMS_WEAPON, CLAN_AMS_WEAPON};
  static const int AMS_FLAGS[] = {IS_ANTI_MISSILE_TECH, CL_ANTI_MISSILE_TECH};

  for (size_t family = 0; family < 2; family++) {
    for (ReplacementWriter writer = REPLACEMENT_WRITER_WEAPON;
         writer <= REPLACEMENT_WRITER_AMMUNITION; writer++) {
      reset_state();
      const int FLAG = *(const int *)checked_storage_at_const(
          AMS_FLAGS, 2, sizeof(*AMS_FLAGS), family);
      const int WEAPON = *(const int *)checked_storage_at_const(
          AMS_WEAPONS, 2, sizeof(*AMS_WEAPONS), family);
      technology_flags = FLAG | CLAN_TECH;
      secondary_technology_flags = STEALTH_ARMOR_TECH;
      *critical_part_address(HEAD, 0) = weapon_equipment_index(WEAPON);

      replace_slot(writer);

      assert((technology_flags & FLAG) == 0);
      assert((technology_flags & CLAN_TECH) != 0);
      assert((secondary_technology_flags & STEALTH_ARMOR_TECH) != 0);
    }
  }
}

static void test_ams_family_duplicates_and_replacements(void) {
  const int SLOT[] = {0};
  const int BOTH_SLOTS[] = {0, 1};

  reset_state();
  technology_flags = IS_ANTI_MISSILE_TECH;
  *critical_part_address(HEAD, 0) = weapon_equipment_index(IS_AMS_WEAPON);
  *critical_part_address(CTORSO, 0) = weapon_equipment_index(IS_AMS_WEAPON);
  btech_admin_special_install(MECH, -1, HEAD, 0, 0);
  assert((technology_flags & IS_ANTI_MISSILE_TECH) != 0);
  btech_admin_ammunition_configure(MECH, TEST_WEAPON, CTORSO, 0, 0, 0);
  assert((technology_flags & IS_ANTI_MISSILE_TECH) == 0);

  reset_state();
  technology_flags = IS_ANTI_MISSILE_TECH | CL_ANTI_MISSILE_TECH;
  *critical_part_address(HEAD, 0) = weapon_equipment_index(IS_AMS_WEAPON);
  *critical_part_address(CTORSO, 0) = weapon_equipment_index(CLAN_AMS_WEAPON);
  install_weapon(HEAD, SLOT, 1);
  assert((technology_flags & IS_ANTI_MISSILE_TECH) == 0);
  assert((technology_flags & CL_ANTI_MISSILE_TECH) != 0);

  reset_state();
  technology_flags = IS_ANTI_MISSILE_TECH;
  weapon_is_ams = true;
  *critical_part_address(HEAD, 0) = weapon_equipment_index(IS_AMS_WEAPON);
  install_weapon(HEAD, SLOT, 1);
  assert((technology_flags & IS_ANTI_MISSILE_TECH) != 0);

  reset_state();
  technology_flags = IS_ANTI_MISSILE_TECH;
  weapon_is_ams = true;
  weapon_is_clan = true;
  *critical_part_address(HEAD, 0) = weapon_equipment_index(IS_AMS_WEAPON);
  install_weapon(HEAD, SLOT, 1);
  assert((technology_flags & IS_ANTI_MISSILE_TECH) == 0);
  assert((technology_flags & CL_ANTI_MISSILE_TECH) != 0);

  reset_state();
  technology_flags = IS_ANTI_MISSILE_TECH;
  *critical_part_address(HEAD, 0) = weapon_equipment_index(IS_AMS_WEAPON);
  *critical_part_address(HEAD, 1) = weapon_equipment_index(IS_AMS_WEAPON);
  install_weapon(HEAD, BOTH_SLOTS, 2);
  assert((technology_flags & IS_ANTI_MISSILE_TECH) == 0);
}

static void test_critical_reset_reconciles_every_special_technology(void) {
  for (size_t index = 0; index < sizeof(EXPECTATIONS) / sizeof(*EXPECTATIONS);
       index++) {
    const SpecialTechnologyExpectation *expectation = checked_storage_at_const(
        EXPECTATIONS, sizeof(EXPECTATIONS) / sizeof(*EXPECTATIONS),
        sizeof(*EXPECTATIONS), index);
    reset_state();
    technology_flags = CLAN_TECH;
    secondary_technology_flags = STEALTH_ARMOR_TECH;
    if (expectation->secondary)
      secondary_technology_flags |= expectation->flag;
    else
      technology_flags |= expectation->flag;

    btech_admin_criticals_reset(MECH);

    const int FLAGS =
        expectation->secondary ? secondary_technology_flags : technology_flags;
    assert((FLAGS & expectation->flag) == 0);
    assert((technology_flags & CLAN_TECH) != 0);
    assert((secondary_technology_flags & STEALTH_ARMOR_TECH) != 0);

    *critical_metadata_address(default_critical_parts, CTORSO, 3) =
        special_equipment_index(expectation->special);
    btech_admin_criticals_reset(MECH);
    const int RESTORED_FLAGS =
        expectation->secondary ? secondary_technology_flags : technology_flags;
    assert((RESTORED_FLAGS & expectation->flag) != 0);
  }
}

static void test_unrelated_manual_metadata_survives_every_writer(void) {
  for (ReplacementWriter writer = REPLACEMENT_WRITER_WEAPON;
       writer <= REPLACEMENT_WRITER_AMMUNITION; writer++) {
    reset_state();
    technology_flags = MASC_TECH | CLAN_TECH;
    secondary_technology_flags = TAG_TECH | STEALTH_ARMOR_TECH;

    replace_slot(writer);

    assert((technology_flags & MASC_TECH) != 0);
    assert((technology_flags & CLAN_TECH) != 0);
    assert((secondary_technology_flags & TAG_TECH) != 0);
    assert((secondary_technology_flags & STEALTH_ARMOR_TECH) != 0);
  }
}

static void test_critical_reset_reconciles_case_and_ams(void) {
  reset_state();
  technology_flags = IS_ANTI_MISSILE_TECH | CL_ANTI_MISSILE_TECH | CLAN_TECH;
  for (int section = 0; section < NUM_SECTIONS; section++)
    *section_configuration_address(section) = CASE_TECH | SECTION_BREACHED;

  btech_admin_criticals_reset(MECH);

  assert((technology_flags & IS_ANTI_MISSILE_TECH) == 0);
  assert((technology_flags & CL_ANTI_MISSILE_TECH) == 0);
  assert((technology_flags & CLAN_TECH) != 0);
  for (int section = 0; section < NUM_SECTIONS; section++)
    assert(*section_configuration_address(section) == SECTION_BREACHED);

  *critical_metadata_address(default_critical_parts, HEAD, 0) =
      weapon_equipment_index(IS_AMS_WEAPON);
  *critical_metadata_address(default_critical_parts, HEAD, 1) =
      weapon_equipment_index(IS_AMS_WEAPON);
  *critical_metadata_address(default_critical_parts, CTORSO, 0) =
      weapon_equipment_index(CLAN_AMS_WEAPON);
  *critical_metadata_address(default_critical_parts, LTORSO, 0) =
      special_equipment_index(CASE);
  btech_admin_criticals_reset(MECH);
  assert((technology_flags & IS_ANTI_MISSILE_TECH) != 0);
  assert((technology_flags & CL_ANTI_MISSILE_TECH) != 0);
  assert((*section_configuration_address(LTORSO) & CASE_TECH) != 0);
  assert((*section_configuration_address(HEAD) & CASE_TECH) == 0);

  unit_class = CLASS_VEH_GROUND;
  *critical_metadata_address(default_critical_parts, HEAD, 2) =
      special_equipment_index(CASE);
  *critical_metadata_address(default_critical_parts, CTORSO, 2) =
      special_equipment_index(CASE);
  btech_admin_criticals_reset(MECH);
  assert((*section_configuration_address(BSIDE) & CASE_TECH) != 0);
  assert((*section_configuration_address(HEAD) & CASE_TECH) == 0);
  assert((*section_configuration_address(CTORSO) & CASE_TECH) == 0);
}

static void test_critical_reset_restores_slots_and_clears_aux_metadata(void) {
  reset_state();
  *critical_metadata_address(default_critical_parts, HEAD, 4) =
      special_equipment_index(ECM);
  seed_slot_metadata(HEAD, 4, 7);

  btech_admin_criticals_reset(MECH);

  assert(*critical_part_address(HEAD, 4) == special_equipment_index(ECM));
  assert(*critical_metadata_address(critical_data_values, HEAD, 4) == 44);
  assert(*critical_metadata_address(critical_fire_modes, HEAD, 4) ==
         REAR_MOUNT);
  assert(*critical_metadata_address(critical_ammo_modes, HEAD, 4) ==
         ARTEMIS_MODE);
  assert(*critical_metadata_address(critical_damage_flags, HEAD, 4) == 0);
  assert(*critical_metadata_address(critical_brands, HEAD, 4) == 0);
  assert(*critical_metadata_address(critical_desired_ammo_sections, HEAD, 4) ==
         -1);
  assert((technology_flags & ECM_TECH) != 0);
}

int main(void) {
  test_reconciles_every_special_technology();
  test_duplicate_and_multi_slot_providers();
  test_case_reconciliation();
  test_ams_and_unrelated_metadata_are_preserved();
  test_removes_each_ams_family_through_every_writer();
  test_ams_family_duplicates_and_replacements();
  test_weapon_install_resets_each_selected_slot_metadata();
  test_ammunition_replacement_resets_slot_and_reconciles_provider();
  test_case_remove_leaves_canonical_empty_slots();
  test_technology_remove_leaves_canonical_empty_slots();
  test_primary_technology_clear_canonicalizes_related_slots();
  test_critical_reset_reconciles_every_special_technology();
  test_critical_reset_reconciles_case_and_ams();
  test_critical_reset_restores_slots_and_clears_aux_metadata();
  test_unrelated_manual_metadata_survives_every_writer();
  return 0;
}
