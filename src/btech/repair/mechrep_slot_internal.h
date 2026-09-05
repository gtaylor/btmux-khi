/* Internal helpers for canonicalizing BattleTech repair slots. */

#pragma once

#include "equipment_types.h"
#include "mech_equipment_api.h"

static inline void
mechrep_slot_auxiliary_metadata_reset(Mech *mech, int section, int slot) {
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
}

static inline void mechrep_slot_empty(Mech *mech, int section, int slot) {
  mech_critical_configure(&(CriticalSlotConfiguration){
      .mech = mech,
      .slot = {.section = section, .critical = slot},
      .part_type = EMPTY,
  });
  mechrep_slot_auxiliary_metadata_reset(mech, section, slot);
}

/** Reconciles equipment-derived flags against the complete critical grid. */
void mechrep_equipment_metadata_reconcile(Mech *mech);

/** Reconciles metadata associated with one part displaced from a slot. */
void mechrep_replaced_part_metadata_reconcile(Mech *mech, int old_part_type,
                                              int old_section);

/** Adds metadata associated with a newly installed special-equipment part. */
void mechrep_special_metadata_add(Mech *mech, int special, int section);
