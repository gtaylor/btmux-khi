/* Shared typed operations used by BattleTech administration front ends. */

#include "btech/repair/mechrep_api.h"
#include "mechrep_slot_internal.h"

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

bool btech_admin_section_has_rear_armor(const Mech *mech, int section) {
  return (bool)(mech_class(mech) == CLASS_MECH &&
                (section == CTORSO || section == LTORSO || section == RTORSO));
}

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

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Slot count and section
// are distinct documented API dimensions.
void btech_admin_weapon_install(Mech *mech, int weapon, const int *slots,
                                size_t slot_count, int section,
                                int fire_modes) {
  int replaced_parts[NUM_CRITICALS] = {[0] = EMPTY};

  for (size_t index = 0; index < slot_count; index++) {
    const int SLOT = *(const int *)checked_storage_at_const(
        slots, slot_count, sizeof(*slots), index);
    *(int *)checked_storage_at(replaced_parts, NUM_CRITICALS,
                               sizeof(*replaced_parts), index) =
        mech_critical_part_type(mech, section, SLOT);
  }
  for (size_t index = 0; index < slot_count; index++) {
    const int SLOT = *(const int *)checked_storage_at_const(
        slots, slot_count, sizeof(*slots), index);
    mech_critical_configure(&(CriticalSlotConfiguration){
        .mech = mech,
        .slot = {.section = section, .critical = SLOT},
        .part_type = weapon_equipment_index(weapon),
        .fire_mode = fire_modes,
    });
    mechrep_slot_auxiliary_metadata_reset(mech, section, SLOT);
  }
  for (size_t index = 0; index < slot_count; index++) {
    mechrep_replaced_part_metadata_reconcile(
        mech,
        *(const int *)checked_storage_at_const(replaced_parts, NUM_CRITICALS,
                                               sizeof(*replaced_parts), index),
        section);
  }
  if (weapon_catalogue_has_special(weapon, AMS))
    mech_technology_flags_add(mech, weapon_catalogue_has_special(weapon, CLAT)
                                        ? CL_ANTI_MISSILE_TECH
                                        : IS_ANTI_MISSILE_TECH);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

void btech_admin_ammunition_configure(Mech *mech, int weapon, int section,
                                      int slot, int fire_modes,
                                      int ammunition_modes) {
  const int OLD_PART_TYPE = mech_critical_part_type(mech, section, slot);
  mech_critical_configure(&(CriticalSlotConfiguration){
      .mech = mech,
      .slot = {.section = section, .critical = slot},
      .part_type = ammunition_equipment_index(weapon),
      .fire_mode = fire_modes,
      .ammo_mode = ammunition_modes,
  });
  mechrep_slot_auxiliary_metadata_reset(mech, section, slot);
  mechrep_replaced_part_metadata_reconcile(mech, OLD_PART_TYPE, section);
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
  const int OLD_PART_TYPE = mech_critical_part_type(mech, section, slot);
  mech_critical_configure(&(CriticalSlotConfiguration){
      .mech = mech,
      .slot = {.section = section, .critical = slot},
      .part_type = special < 0 ? EMPTY : special_equipment_index(special),
      .data = data,
  });
  mechrep_slot_auxiliary_metadata_reset(mech, section, slot);
  mechrep_replaced_part_metadata_reconcile(mech, OLD_PART_TYPE, section);
  mechrep_special_metadata_add(mech, special, section);
  if (special == SPLIT_CRIT_LEFT || special == SPLIT_CRIT_RIGHT) {
    mech_critical_data_set(mech, section, slot,
                           mech_critical_data(mech, section, slot) - 1);
  }
}

static void remove_critical_type(Mech *mech, int part_type) {
  for (int section = 0; section < NUM_SECTIONS; section++)
    for (int critical = 0; critical < NUM_CRITICALS; critical++)
      if (mech_critical_part_type(mech, section, critical) == part_type)
        mechrep_slot_empty(mech, section, critical);
}

void btech_admin_case_remove(Mech *mech) {
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
    if (enabled) {
      mech_technology_flags_add(mech, technology);
    } else {
      const int CURRENT_TECHNOLOGY = mech_technology_flags(mech);
      if ((technology & TRIPLE_MYOMER_TECH) != 0 &&
          (CURRENT_TECHNOLOGY & TRIPLE_MYOMER_TECH) != 0)
        remove_critical_type(mech,
                             special_equipment_index(TRIPLE_STRENGTH_MYOMER));
      if ((technology & MASC_TECH) != 0 &&
          (CURRENT_TECHNOLOGY & MASC_TECH) != 0)
        remove_critical_type(mech, special_equipment_index(MASC));
      mech_technology_flags_remove(mech, technology);
    }
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
    btech_admin_case_remove(mech);
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
