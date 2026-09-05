/* Shared typed operations used by BattleTech administration front ends. */

#include "btech/repair/mechrep_api.h"

#include <stddef.h>

#include "btech/unit/equipment_types.h"
#include "btech/unit/mech_classification_api.h"
#include "btech/unit/mech_equipment_api.h"
#include "btech/unit/mech_specification_api.h"
#include "btech/unit/mech_status_types.h"
#include "btech/unit/mech_utils_api.h"
#include "btech/unit/section_types.h"
#include "btech/unit/template_api.h"
#include "btech/unit/weapon_catalogue_api.h"
#include "mux/support/checked_storage.h"

void btech_admin_armor_set(Mech *mech, int section, bool has_armor, int armor,
                           bool has_internal, int internal, bool has_rear,
                           int rear) {
  if (has_armor) {
    mech_section_armor_set(mech, section, armor);
    mech_section_original_armor_set(mech, section, armor);
  }
  if (has_internal) {
    mech_section_internal_set(mech, section, internal);
    mech_section_original_internal_set(mech, section, internal);
  }
  if (has_rear) {
    mech_section_rear_armor_set(mech, section, rear);
    mech_section_original_rear_armor_set(mech, section, rear);
  }
}

void btech_admin_weapon_install(Mech *mech, int weapon, const int *slots,
                                size_t slot_count, int section,
                                int fire_modes) {
  for (size_t index = 0; index < slot_count; index++) {
    mech_critical_configure(&(CriticalSlotConfiguration){
        .mech = mech,
        .slot = {.section = section,
                 .critical = *(const int *)checked_storage_at_const(
                     slots, slot_count, sizeof(*slots), index)},
        .part_type = weapon_equipment_index(weapon),
        .fire_mode = fire_modes,
    });
  }
  if (weapon_catalogue_has_special(weapon, AMS))
    mech_technology_flags_add(mech, weapon_catalogue_has_special(weapon, CLAT)
                                        ? CL_ANTI_MISSILE_TECH
                                        : IS_ANTI_MISSILE_TECH);
}

void btech_admin_ammunition_configure(Mech *mech, int weapon, int section,
                                      int slot, int fire_modes,
                                      int ammunition_modes) {
  mech_critical_configure(&(CriticalSlotConfiguration){
      .mech = mech,
      .slot = {.section = section, .critical = slot},
      .part_type = ammunition_equipment_index(weapon),
      .fire_mode = fire_modes,
      .ammo_mode = ammunition_modes,
  });
  mech_critical_data_set(mech, section, slot, full_ammo(mech, section, slot));
}

void btech_admin_ammunition_restock(Mech *mech, int section, int slot) {
  mech_critical_data_set(mech, section, slot, full_ammo(mech, section, slot));
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Distinct documented mode
// sets.
bool btech_admin_weapon_modes_set(Mech *mech, int weapon_number, int fire_modes,
                                  int ammunition_modes) {
  const WeaponNumberLookupResult LOOKUP = weapon_number_find(
      &(WeaponNumberLookupRequest){.mech = mech, .number = weapon_number});
  if (!LOOKUP.found)
    return false;
  mech_critical_fire_mode_set(mech, LOOKUP.slot.section, LOOKUP.slot.critical,
                              fire_modes);
  mech_critical_ammo_mode_set(mech, LOOKUP.slot.section, LOOKUP.slot.critical,
                              ammunition_modes);
  return true;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Section and value follow
// the repair request shape.
void btech_admin_repair(Mech *mech, BtechAdminRepairKind kind, int section,
                        int value) {
  switch (kind) {
  case BTECH_ADMIN_REPAIR_ARMOR:
    mech_section_armor_set(mech, section, value);
    break;
  case BTECH_ADMIN_REPAIR_INTERNAL:
    mech_section_internal_set(mech, section, value);
    break;
  case BTECH_ADMIN_REPAIR_REAR_ARMOR:
    mech_section_rear_armor_set(mech, section, value);
    break;
  case BTECH_ADMIN_REPAIR_CRITICAL:
    mech_repair_part(mech, section, value);
    break;
  case BTECH_ADMIN_REPAIR_REATTACH:
    mech_re_attach(mech, section);
    break;
  }
}
// NOLINTEND(bugprone-easily-swappable-parameters)

void btech_admin_special_install(Mech *mech, int special, int section, int slot,
                                 int data) {
  mech_critical_configure(&(CriticalSlotConfiguration){
      .mech = mech,
      .slot = {.section = section, .critical = slot},
      .part_type = special < 0 ? EMPTY : special_equipment_index(special),
      .data = data,
  });
  mech_critical_damage_flags_set(mech, section, slot, 0);
  mech_critical_temporary_failure_set(&(CriticalSlotFailureSet){
      .mech = mech,
      .slot = {.section = section, .critical = slot},
      .failure = 0,
  });
  mech_critical_brand_set(&(CriticalSlotBrandSet){
      .mech = mech,
      .slot = {.section = section, .critical = slot},
      .brand = 0,
  });
  mech_critical_desired_ammo_section_set(mech, section, slot, -1);
  if (special == CASE) {
    mech_section_configuration_add(
        mech, mech_class(mech) == CLASS_VEH_GROUND ? BSIDE : section,
        CASE_TECH);
  } else if (special == TRIPLE_STRENGTH_MYOMER) {
    mech_technology_flags_add(mech, TRIPLE_MYOMER_TECH);
  } else if (special == MASC) {
    mech_technology_flags_add(mech, MASC_TECH);
  } else if (special == C3_MASTER) {
    mech_technology_flags_add(mech, C3_MASTER_TECH);
  } else if (special == C3_SLAVE) {
    mech_technology_flags_add(mech, C3_SLAVE_TECH);
  } else if (special == ARTEMIS_IV) {
    mech_technology_flags_add(mech, ARTEMIS_IV_TECH);
  } else if (special == ECM) {
    mech_technology_flags_add(mech, ECM_TECH);
  } else if (special == ANGELECM) {
    mech_technology_flags_secondary_add(mech, ANGEL_ECM_TECH);
  } else if (special == BEAGLE_PROBE) {
    mech_technology_flags_add(mech, BEAGLE_PROBE_TECH);
  } else if (special == LIGHT_BAP) {
    mech_technology_flags_add(mech, LIGHT_BAP_TECH);
  } else if (special == TAG) {
    mech_technology_flags_secondary_add(mech, TAG_TECH);
  } else if (special == C3I) {
    mech_technology_flags_secondary_add(mech, C3I_TECH);
  } else if (special == BLOODHOUND_PROBE) {
    mech_technology_flags_secondary_add(mech, BLOODHOUND_PROBE_TECH);
  } else if (special == TARGETING_COMPUTER) {
    mech_technology_flags_secondary_add(mech, TCOMP_TECH);
  }
  if (special == SPLIT_CRIT_LEFT || special == SPLIT_CRIT_RIGHT) {
    mech_critical_data_set(mech, section, slot,
                           mech_critical_data(mech, section, slot) - 1);
  }
}

static void remove_critical_type(Mech *mech, int part_type) {
  for (int section = 0; section < NUM_SECTIONS; section++)
    for (int critical = 0; critical < NUM_CRITICALS; critical++)
      if (mech_critical_part_type(mech, section, critical) == part_type)
        mech_critical_part_type_set(mech, section, critical, EMPTY);
}

static void remove_case_technology(Mech *mech) {
  remove_critical_type(mech, special_equipment_index(CASE));
  for (int section = 0; section < NUM_SECTIONS; section++)
    mech_section_configuration_remove(mech, section, CASE_TECH);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): The enum group qualifies
// the technology bit.
void btech_admin_technology_set(Mech *mech, BtechAdminTechnologyGroup group,
                                int technology, bool enabled) {
  switch (group) {
  case BTECH_ADMIN_TECHNOLOGY_PRIMARY:
    if (enabled)
      mech_technology_flags_add(mech, technology);
    else
      mech_technology_flags_remove(mech, technology);
    break;
  case BTECH_ADMIN_TECHNOLOGY_SECONDARY:
    if (enabled)
      mech_technology_flags_secondary_add(mech, technology);
    else
      mech_technology_flags_secondary_remove(mech, technology);
    break;
  case BTECH_ADMIN_TECHNOLOGY_INFANTRY:
    if (enabled)
      mech_infantry_technology_flags_add(mech, technology);
    else
      mech_infantry_technology_flags_set(
          mech, mech_infantry_technology_flags(mech) & ~technology);
    break;
  }
}
// NOLINTEND(bugprone-easily-swappable-parameters)

void btech_admin_technologies_clear(Mech *mech,
                                    BtechAdminTechnologyGroup group) {
  switch (group) {
  case BTECH_ADMIN_TECHNOLOGY_PRIMARY:
    remove_case_technology(mech);
    remove_critical_type(mech, special_equipment_index(TRIPLE_STRENGTH_MYOMER));
    remove_critical_type(mech, special_equipment_index(MASC));
    mech_technology_flags_set(mech, 0);
    break;
  case BTECH_ADMIN_TECHNOLOGY_SECONDARY:
    mech_technology_flags_secondary_set(mech, 0);
    break;
  case BTECH_ADMIN_TECHNOLOGY_INFANTRY:
    mech_infantry_technology_flags_set(mech, 0);
    break;
  }
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Both named cargo
// dimensions are required.
void btech_admin_cargo_set(Mech *mech, int space, int maximum_tons) {
  mech_cargo_space_set(mech, space * 50);
  mech_carrier_maximum_tonnage_set(mech, maximum_tons);
}
// NOLINTEND(bugprone-easily-swappable-parameters)
