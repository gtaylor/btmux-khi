/* Reconciles metadata derived from the complete critical-slot grid. */

#include "mechrep_slot_internal.h"

#include <stddef.h>

#include "btech/unit/equipment_types.h"
#include "btech/unit/mech_classification_api.h"
#include "btech/unit/mech_equipment_api.h"
#include "btech/unit/mech_specification_api.h"
#include "btech/unit/mech_status_types.h"
#include "btech/unit/section_types.h"
#include "btech/unit/weapon_catalogue_api.h"
#include "mux/support/checked_storage.h"

typedef enum SpecialTechnologyGroup : int {
  SPECIAL_TECHNOLOGY_PRIMARY,
  SPECIAL_TECHNOLOGY_SECONDARY,
} SpecialTechnologyGroup;

typedef struct SpecialTechnology {
  int special;
  SpecialTechnologyGroup group;
  int flag;
} SpecialTechnology;

static const SpecialTechnology SPECIAL_TECHNOLOGIES[] = {
    {TRIPLE_STRENGTH_MYOMER, SPECIAL_TECHNOLOGY_PRIMARY, TRIPLE_MYOMER_TECH},
    {MASC, SPECIAL_TECHNOLOGY_PRIMARY, MASC_TECH},
    {C3_MASTER, SPECIAL_TECHNOLOGY_PRIMARY, C3_MASTER_TECH},
    {C3_SLAVE, SPECIAL_TECHNOLOGY_PRIMARY, C3_SLAVE_TECH},
    {ARTEMIS_IV, SPECIAL_TECHNOLOGY_PRIMARY, ARTEMIS_IV_TECH},
    {ECM, SPECIAL_TECHNOLOGY_PRIMARY, ECM_TECH},
    {BEAGLE_PROBE, SPECIAL_TECHNOLOGY_PRIMARY, BEAGLE_PROBE_TECH},
    {LIGHT_BAP, SPECIAL_TECHNOLOGY_PRIMARY, LIGHT_BAP_TECH},
    {ANGELECM, SPECIAL_TECHNOLOGY_SECONDARY, ANGEL_ECM_TECH},
    {TAG, SPECIAL_TECHNOLOGY_SECONDARY, TAG_TECH},
    {C3I, SPECIAL_TECHNOLOGY_SECONDARY, C3I_TECH},
    {BLOODHOUND_PROBE, SPECIAL_TECHNOLOGY_SECONDARY, BLOODHOUND_PROBE_TECH},
    {TARGETING_COMPUTER, SPECIAL_TECHNOLOGY_SECONDARY, TCOMP_TECH},
};

static bool critical_type_exists(Mech *mech, int part_type) {
  for (int section = 0; section < NUM_SECTIONS; section++)
    for (int critical = 0;
         critical < mech_section_critical_count(mech, section); critical++)
      if (mech_critical_part_type(mech, section, critical) == part_type)
        return true;
  return false;
}

static int case_configuration_section(const Mech *mech, int section) {
  return mech_class(mech) == CLASS_VEH_GROUND ? BSIDE : section;
}

static bool case_exists_for_configuration(Mech *mech,
                                          int configuration_section) {
  const int CASE_PART_TYPE = special_equipment_index(CASE);
  for (int section = 0; section < NUM_SECTIONS; section++) {
    for (int critical = 0;
         critical < mech_section_critical_count(mech, section); critical++)
      if (mech_critical_part_type(mech, section, critical) == CASE_PART_TYPE &&
          case_configuration_section(mech, section) == configuration_section)
        return true;
  }
  return false;
}

static bool anti_missile_family_exists(Mech *mech, bool clan) {
  for (int section = 0; section < NUM_SECTIONS; section++) {
    for (int critical = 0;
         critical < mech_section_critical_count(mech, section); critical++) {
      const int PART_TYPE = mech_critical_part_type(mech, section, critical);
      if (!equipment_is_weapon(PART_TYPE))
        continue;
      const int WEAPON = weapon_from_equipment_index(PART_TYPE);
      if (weapon_catalogue_is_anti_missile(WEAPON) &&
          weapon_catalogue_has_special(WEAPON, CLAT) == clan)
        return true;
    }
  }
  return false;
}

static void technology_flag_reconcile(Mech *mech,
                                      const SpecialTechnology *technology) {
  const bool EXISTS =
      critical_type_exists(mech, special_equipment_index(technology->special));
  if (technology->group == SPECIAL_TECHNOLOGY_PRIMARY) {
    if (EXISTS)
      mech_technology_flags_add(mech, technology->flag);
    else
      mech_technology_flags_remove(mech, technology->flag);
    return;
  }
  if (EXISTS)
    mech_technology_flags_secondary_add(mech, technology->flag);
  else
    mech_technology_flags_secondary_remove(mech, technology->flag);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Part and section have
// distinct equipment-model meanings.
void mechrep_replaced_part_metadata_reconcile(Mech *mech, int old_part_type,
                                              int old_section) {
  if (equipment_is_weapon(old_part_type)) {
    const int WEAPON = weapon_from_equipment_index(old_part_type);
    if (!weapon_catalogue_is_anti_missile(WEAPON))
      return;
    const bool CLAN = weapon_catalogue_has_special(WEAPON, CLAT);
    if (!anti_missile_family_exists(mech, CLAN))
      mech_technology_flags_remove(mech, CLAN ? CL_ANTI_MISSILE_TECH
                                              : IS_ANTI_MISSILE_TECH);
    return;
  }

  if (old_part_type == special_equipment_index(CASE)) {
    const int CONFIGURATION_SECTION =
        case_configuration_section(mech, old_section);
    if (!case_exists_for_configuration(mech, CONFIGURATION_SECTION))
      mech_section_configuration_remove(mech, CONFIGURATION_SECTION, CASE_TECH);
    return;
  }

  for (size_t index = 0;
       index < sizeof(SPECIAL_TECHNOLOGIES) / sizeof(*SPECIAL_TECHNOLOGIES);
       index++) {
    const SpecialTechnology *technology = checked_storage_at_const(
        SPECIAL_TECHNOLOGIES,
        sizeof(SPECIAL_TECHNOLOGIES) / sizeof(*SPECIAL_TECHNOLOGIES),
        sizeof(*SPECIAL_TECHNOLOGIES), index);
    if (old_part_type != special_equipment_index(technology->special) ||
        critical_type_exists(mech, old_part_type))
      continue;
    if (technology->group == SPECIAL_TECHNOLOGY_PRIMARY)
      mech_technology_flags_remove(mech, technology->flag);
    else
      mech_technology_flags_secondary_remove(mech, technology->flag);
    return;
  }
}

// NOLINTEND(bugprone-easily-swappable-parameters)

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Special and section have
// distinct equipment-model meanings.
void mechrep_special_metadata_add(Mech *mech, int special, int section) {
  if (special == CASE) {
    mech_section_configuration_add(
        mech, case_configuration_section(mech, section), CASE_TECH);
    return;
  }

  for (size_t index = 0;
       index < sizeof(SPECIAL_TECHNOLOGIES) / sizeof(*SPECIAL_TECHNOLOGIES);
       index++) {
    const SpecialTechnology *technology = checked_storage_at_const(
        SPECIAL_TECHNOLOGIES,
        sizeof(SPECIAL_TECHNOLOGIES) / sizeof(*SPECIAL_TECHNOLOGIES),
        sizeof(*SPECIAL_TECHNOLOGIES), index);
    if (special != technology->special)
      continue;
    if (technology->group == SPECIAL_TECHNOLOGY_PRIMARY)
      mech_technology_flags_add(mech, technology->flag);
    else
      mech_technology_flags_secondary_add(mech, technology->flag);
    return;
  }
}
// NOLINTEND(bugprone-easily-swappable-parameters)

void mechrep_equipment_metadata_reconcile(Mech *mech) {
  for (size_t index = 0;
       index < sizeof(SPECIAL_TECHNOLOGIES) / sizeof(*SPECIAL_TECHNOLOGIES);
       index++) {
    technology_flag_reconcile(
        mech, checked_storage_at_const(SPECIAL_TECHNOLOGIES,
                                       sizeof(SPECIAL_TECHNOLOGIES) /
                                           sizeof(*SPECIAL_TECHNOLOGIES),
                                       sizeof(*SPECIAL_TECHNOLOGIES), index));
  }

  for (int section = 0; section < NUM_SECTIONS; section++) {
    if (case_exists_for_configuration(mech, section))
      mech_section_configuration_add(mech, section, CASE_TECH);
    else
      mech_section_configuration_remove(mech, section, CASE_TECH);
  }

  const bool IS_AMS = anti_missile_family_exists(mech, false);
  const bool CLAN_AMS = anti_missile_family_exists(mech, true);
  if (IS_AMS)
    mech_technology_flags_add(mech, IS_ANTI_MISSILE_TECH);
  else
    mech_technology_flags_remove(mech, IS_ANTI_MISSILE_TECH);
  if (CLAN_AMS)
    mech_technology_flags_add(mech, CL_ANTI_MISSILE_TECH);
  else
    mech_technology_flags_remove(mech, CL_ANTI_MISSILE_TECH);
}
