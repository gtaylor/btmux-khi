/* Declares the BattleTech mechrep API. */

#include "mux/server/platform.h"

#include <stdbool.h>
#include <stddef.h>

#pragma once

typedef struct BtechContext BtechContext;

typedef enum BtechAdminRepairKind : int {
  BTECH_ADMIN_REPAIR_ARMOR,
  BTECH_ADMIN_REPAIR_INTERNAL,
  BTECH_ADMIN_REPAIR_REAR_ARMOR,
  BTECH_ADMIN_REPAIR_CRITICAL,
  BTECH_ADMIN_REPAIR_REATTACH,
} BtechAdminRepairKind;

typedef enum BtechAdminTechnologyGroup : int {
  BTECH_ADMIN_TECHNOLOGY_PRIMARY,
  BTECH_ADMIN_TECHNOLOGY_SECONDARY,
  BTECH_ADMIN_TECHNOLOGY_INFANTRY,
} BtechAdminTechnologyGroup;

/** Loads a template and resets transient unit state. */
bool btech_admin_template_load(DbRef player, Mech *mech, const char *reference);
/** Reloads the unit's model template and resets transient unit state. */
bool btech_admin_template_restore(DbRef player, Mech *mech);
/** Saves a unit beneath the configured template directory. */
bool btech_admin_template_save(DbRef player, Mech *mech, const char *reference);
/** Configures current and original armor values for a section. */
void btech_admin_armor_set(Mech *mech, int section, bool has_armor, int armor,
                           bool has_internal, int internal, bool has_rear,
                           int rear);
/**
 * Reports whether a section has combat-effective rear armor.
 *
 * Combat rear-armor consumption is authoritative: only center, left, and right
 * torso sections on a Mech consume rear armor. Other stored rear values remain
 * available to compatibility-oriented configuration and scheduled repair code.
 *
 * @param mech Unit whose class defines the section layout.
 * @param section Zero-based class-specific section index.
 * @return True only for a Mech center, left, or right torso.
 */
bool btech_admin_section_has_rear_armor(const Mech *mech, int section);
/**
 * Restores default critical slots, clears auxiliary slot metadata, and
 * reconciles equipment-derived technology and section flags.
 */
void btech_admin_criticals_reset(Mech *mech);
/**
 * Installs a weapon, clears replaced-slot damage and selection metadata, and
 * reconciles metadata derived from replaced parts.
 *
 * Slots must be unique, valid for the section, and contain no more than
 * `NUM_CRITICALS` entries. Callers must validate the complete request before
 * invoking this operation.
 */
void btech_admin_weapon_install(Mech *mech, int weapon, const int *slots,
                                size_t slot_count, int section, int fire_modes);
/**
 * Configures ammunition, clears replaced-slot damage and selection metadata,
 * refills the slot, and reconciles metadata derived from the replaced part.
 */
void btech_admin_ammunition_configure(Mech *mech, int weapon, int section,
                                      int slot, int fire_modes,
                                      int ammunition_modes);
/** Restocks a validated ammunition critical slot. */
void btech_admin_ammunition_restock(Mech *mech, int section, int slot);
/** Replaces the modes on a mounted weapon's primary critical slot. */
bool btech_admin_weapon_modes_set(Mech *mech, int weapon_number, int fire_modes,
                                  int ammunition_modes);
/** Performs an immediate administrative repair. */
void btech_admin_repair(Mech *mech, BtechAdminRepairKind kind, int section,
                        int value);
/**
 * Configures a special-equipment or empty critical slot and reconciles
 * metadata derived from the replaced part and newly installed special.
 */
void btech_admin_special_install(Mech *mech, int special, int section, int slot,
                                 int data);
/** Removes every CASE critical and CASE section-configuration flag. */
void btech_admin_case_remove(Mech *mech);
/** Adds or removes one technology bit, including related TSM/MASC criticals. */
void btech_admin_technology_set(Mech *mech, BtechAdminTechnologyGroup group,
                                int technology, bool enabled);
/** Clears technologies in the selected group and related equipment. */
void btech_admin_technologies_clear(Mech *mech,
                                    BtechAdminTechnologyGroup group);
/** Sets cargo capacity using public tons and carrier tonnage units. */
void btech_admin_cargo_set(Mech *mech, int space, int maximum_tons);
/** Sets unit class and its canonical default movement type. */
void btech_admin_unit_type_set(Mech *mech, int unit_type);
/** Sets the unit movement type. */
void btech_admin_movement_type_set(Mech *mech, int movement_type);
/** Sets maximum speed in movement points. */
void btech_admin_max_speed_set(Mech *mech, float movement_points);
/** Sets jump speed in movement points. */
void btech_admin_jump_speed_set(Mech *mech, float movement_points);
/** Sets heat sink count. */
void btech_admin_heat_sinks_set(Mech *mech, int count);
/** Sets the long-range sensor range. */
void btech_admin_long_range_set(Mech *mech, int range);
/** Sets the tactical sensor range. */
void btech_admin_tactical_range_set(Mech *mech, int range);
/** Sets the scan range. */
void btech_admin_scan_range_set(Mech *mech, int range);
/** Sets radio quality and derives its configuration and range. */
void btech_admin_radio_quality_set(Mech *mech, int quality);
/** Sets radio range. */
void btech_admin_radio_range_set(Mech *mech, int range);

/* mechrep.c */
void mechrep_rresetcrits(DbRef player, void *data, char *buffer);
void mechrep_rdisplaysection(DbRef player, void *data, char *buffer);
void mechrep_rsetradio(DbRef player, void *data, char *buffer);
void mechrep_rsettype(DbRef player, void *data, char *buffer);
void mechrep_rsetspeed(DbRef player, void *data, char *buffer);
void mechrep_rsetjumpspeed(DbRef player, void *data, char *buffer);
void mechrep_rsetheatsinks(DbRef player, void *data, char *buffer);
void mechrep_rsetlrsrange(DbRef player, void *data, char *buffer);
void mechrep_rsettacrange(DbRef player, void *data, char *buffer);
void mechrep_rsetscanrange(DbRef player, void *data, char *buffer);
void mechrep_rsetradiorange(DbRef player, void *data, char *buffer);
void mechrep_rsettons(DbRef player, void *data, char *buffer);
void mechrep_rsetmove(DbRef player, void *data, char *buffer);
void mechrep_rloadnew(DbRef player, void *data, char *buffer);
Mech *load_refmech(BtechContext *context, const char *reference);
void mech_reference_cache_destroy(BtechContext *context);
void mechrep_rrestore(DbRef player, void *data, char *buffer);
void mechrep_rsavetemp2(DbRef player, void *data, char *buffer);
void mechrep_rsetarmor(DbRef player, void *data, char *buffer);
void mechrep_raddweap(DbRef player, void *data, char *buffer);
void mechrep_rreload(DbRef player, void *data, char *buffer);
void mechrep_rrepair(DbRef player, void *data, char *buffer);
void mechrep_raddspecial(DbRef player, void *data, char *buffer);
void mechrep_rrestock(DbRef player, void *data, char *buffer);
void mechrep_rfiremode(DbRef player, void *data, char *buffer);
const char *techstatus_func(Mech *mech);
void mechrep_rshowtech(DbRef player, void *data, char *buffer);
void mechrep_gettechstring(Mech *mech, char *buffer);
void mechrep_rdeltech(DbRef player, void *data, char *buffer);
void mechrep_raddtech(DbRef player, void *data, char *buffer);
void mechrep_rdelinftech(DbRef player, void *data, char *buffer);
void mechrep_raddinftech(DbRef player, void *data, char *buffer);
void mechrep_setcargospace(DbRef player, void *data, char *buffer);
void invalid_section(DbRef player, Mech *mech);
