/* Implements BattleTech repair mechanics for mechrep. */

#include <limits.h>
#include <math.h>
#include <stdarg.h>

#include "btconfig.h"
#include "btech/context.h"
#include "btech_event.h"
#include "command_handlers_api.h"
#include "context_internal.h" // IWYU pragma: keep
#include "equipment_types.h"
#include "map_terrain.h"    // IWYU pragma: keep
#include "mech_lifecycle.h" // IWYU pragma: keep
#include "mux/server/game.h"
#include "mux/server/platform.h"
#include "mux/support/checked_storage.h"
#include "registry_api.h"
#include "repair_job.h"

#include "checked_conversion.h"

#include "mech_build_api.h"
#include "mech_classification_api.h"
#include "mech_electronics_api.h"
#include "mech_specification_api.h"
#include "mech_status_api.h"
#include "mech_utils_api.h"
#include "mechrep_api.h"
#include "mechrep_slot_internal.h"
#include "mux/support/stringutil.h"
#include "section_types.h"
#include "special_object.h"
#include "template_api.h"

void btech_admin_criticals_reset(Mech *mech) {
  for (int section = 0; section < NUM_SECTIONS; section++) {
    fill_default_criticals(mech, section);
    for (int slot = 0; slot < NUM_CRITICALS; slot++)
      mechrep_slot_auxiliary_metadata_reset(mech, section, slot);
  }
  mechrep_equipment_metadata_reconcile(mech);
}

void btech_admin_radio_quality_set(Mech *mech, int quality) {
  mech_radio_quality_set(mech, quality);
  mech_radio_configuration_set(mech, generic_radio_type(quality, 0));
  mech_radio_range_set(
      mech,
      clamp_float_to_int(DEFAULT_RADIORANGE * generic_radio_multiplier(mech)));
}

void btech_admin_unit_type_set(Mech *mech, int unit_type) {
  mech_class_set(mech, (UnitClass)unit_type);
  switch (unit_type) {
  case CLASS_MECH:
  case CLASS_BSUIT:
    mech_movement_type_set(mech, MOVE_BIPED);
    break;
  case CLASS_VTOL:
    mech_movement_type_set(mech, MOVE_VTOL);
    break;
  case CLASS_AERO:
  case CLASS_DS:
  case CLASS_SPHEROID_DS:
    mech_movement_type_set(mech, MOVE_FLY);
    break;
  default:
    break;
  }
}

void btech_admin_movement_type_set(Mech *mech, int movement_type) {
  mech_movement_type_set(mech, (MechMovementType)movement_type);
}

void btech_admin_max_speed_set(Mech *mech, float movement_points) {
  mech_maximum_speed_set(mech, movement_points * KPH_PER_MP);
}

void btech_admin_jump_speed_set(Mech *mech, float movement_points) {
  mech_jump_speed_set(mech, movement_points * KPH_PER_MP);
}

void btech_admin_heat_sinks_set(Mech *mech, int count) {
  mech_heat_sink_count_set(mech, count);
}

void btech_admin_long_range_set(Mech *mech, int range) {
  mech_long_range_sensor_range_set(mech, range);
}

void btech_admin_tactical_range_set(Mech *mech, int range) {
  mech_tactical_range_set(mech, range);
}

void btech_admin_scan_range_set(Mech *mech, int range) {
  mech_scanner_range_set(mech, range);
}

void btech_admin_radio_range_set(Mech *mech, int range) {
  mech_radio_range_set(mech, range);
}

/* Selectors */

/*--------------------------------------------------------------------------*/

/* Code Begins                                                              */

/*--------------------------------------------------------------------------*/

/* With cap R means restricted command */

void mechrep_rresetcrits(DbRef player, void *data,
                         char *buffer [[maybe_unused]]) {
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  mecha_notify(btech_context_evaluation(context), player,
               "Default criticals set!");
  btech_admin_criticals_reset(mech);
}

void mechrep_rdisplaysection(DbRef player, void *data, char *buffer) {
  char *args[1];
  int index;

  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (mech_parseattributes(buffer, args, 1) != 1) {
    mecha_notify(btech_context_evaluation(context), player,
                 "You must specify a section to list the criticals for!");
    return;
  }
  index = armor_section_from_string(mech_class(mech), mech_movement_type(mech),
                                    args[0]);
  if (index == -1) {
    mecha_notify(btech_context_evaluation(context), player, "Invalid section!");
    return;
  }
  critical_status(btech_context_evaluation(context), player, mech, index);
}

void mechrep_rsetradio(DbRef player, void *data, char *buffer) {
  char *args[2];
  int i;

  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  switch (mech_parseattributes(buffer, args, 2)) {
  case 0:
    mecha_notify(btech_context_evaluation(context), player,
                 "This remains to be done [showing of stuff when no args]");
    return;
  case 2:
    mecha_notify(btech_context_evaluation(context), player,
                 "Too many args, unable to cope().");
    return;
  }
  if (!parse_int_checked(args[0], &i)) {
    mecha_notify(btech_context_evaluation(context), player,
                 "Invalid radio level!");
    return;
  }
  i = bounded(1, i, 5);
  notify_printf(btech_context_evaluation(context), player,
                "Radio level set to %d.", i);
  btech_admin_radio_quality_set(mech, i);
  notify_printf(btech_context_evaluation(context), player,
                "Number of freqs: %d  Extra stuff: %d",
                mech_radio_configuration(mech) % 16,
                (mech_radio_configuration(mech) / 16) * 16);
  notify_printf(btech_context_evaluation(context), player,
                "Radio range set to %d.", mech_radio_range(mech));
}

void mechrep_rsettype(DbRef player, void *data, char *buffer) {
  char *args[1];

  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (mech_parseattributes(buffer, args, 1) != 1) {
    mecha_notify(btech_context_evaluation(context), player,
                 "Invalid number of arguments!");
    return;
  }
  char *movement =
      *(char **)checked_storage_at((void *)args, 1, sizeof(*args), 0);
  switch (ascii_to_upper(*movement)) {
  case 'M':
    btech_admin_unit_type_set(mech, CLASS_MECH);
    mecha_notify(btech_context_evaluation(context), player, "Type set to MECH");
    break;
  case 'Q':
    btech_admin_unit_type_set(mech, CLASS_MECH);
    btech_admin_movement_type_set(mech, MOVE_QUAD);
    mecha_notify(btech_context_evaluation(context), player, "Type set to QUAD");
    break;
  case 'G':
    btech_admin_unit_type_set(mech, CLASS_VEH_GROUND);
    mecha_notify(btech_context_evaluation(context), player,
                 "Type set to VEHICLE");
    break;
  case 'V':
    btech_admin_unit_type_set(mech, CLASS_VTOL);
    mecha_notify(btech_context_evaluation(context), player, "Type set to VTOL");
    break;
  case 'N':
    btech_admin_unit_type_set(mech, CLASS_VEH_NAVAL);
    mecha_notify(btech_context_evaluation(context), player,
                 "Type set to NAVAL");
    break;
  case 'A':
    btech_admin_unit_type_set(mech, CLASS_AERO);
    mecha_notify(btech_context_evaluation(context), player,
                 "Type set to AeroSpace");
    break;
  case 'D':
    btech_admin_unit_type_set(mech, CLASS_DS);
    mecha_notify(btech_context_evaluation(context), player,
                 "Type set to DropShip");
    break;
  case 'S':
    btech_admin_unit_type_set(mech, CLASS_SPHEROID_DS);
    mecha_notify(btech_context_evaluation(context), player,
                 "Type set to SpheroidDropship");
    break;
  case 'B':
    btech_admin_unit_type_set(mech, CLASS_BSUIT);
    mecha_notify(btech_context_evaluation(context), player,
                 "Type set to BattleSuit");
    break;
  default:
    mecha_notify(btech_context_evaluation(context), player,
                 "Types are: MECH, GROUND, VTOL, NAVAL, AERO, DROPSHIP and "
                 "SPHEROIDDROPSHIP");
    break;
  }
}

static bool parse_repair_float(BtechContext *context, DbRef player,
                               char *buffer, const char *name, float *value) {
  char *args[2];
  if (mech_parseattributes(buffer, args, 2) != 1 ||
      !parse_float_checked(args[0], value)) {
    notify_printf(btech_context_evaluation(context), player,
                  "Invalid value for Set%s!", name);
    return false;
  }
  return true;
}

static bool parse_repair_int(BtechContext *context, DbRef player, char *buffer,
                             const char *name, int *value) {
  char *args[2];
  if (mech_parseattributes(buffer, args, 2) != 1 ||
      !parse_int_checked(args[0], value)) {
    notify_printf(btech_context_evaluation(context), player,
                  "Invalid value for Set%s!", name);
    return false;
  }
  return true;
}

static bool validate_repair_int_range(BtechContext *context, DbRef player,
                                      const char *name, int value, int minimum,
                                      int maximum) {
  if (value >= minimum && value <= maximum)
    return true;

  notify_printf(btech_context_evaluation(context), player,
                "Invalid value for Set%s!", name);
  return false;
}

static void notify_repair_float(BtechContext *context, DbRef player,
                                const char *name, float value) {
  notify_printf(btech_context_evaluation(context), player,
                "%s changed to %.2f.", name, (double)value);
}

static void notify_repair_int(BtechContext *context, DbRef player,
                              const char *name, int value) {
  notify_printf(btech_context_evaluation(context), player, "%s changed to %d.",
                name, value);
}

void mechrep_rsetspeed(DbRef player, void *data, char *buffer) {
  float value;
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (!parse_repair_float(context, player, buffer, "Maxspeed", &value))
    return;
  value *= KPH_PER_MP;
  if (value < 0.0F || !isfinite(value)) {
    notify_printf(btech_context_evaluation(context), player,
                  "Invalid value for SetMaxspeed!");
    return;
  }
  btech_admin_max_speed_set(mech, value / KPH_PER_MP);
  notify_repair_float(context, player, "Maxspeed", mech_maximum_speed(mech));
}

void mechrep_rsetjumpspeed(DbRef player, void *data, char *buffer) {
  float value;
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (!parse_repair_float(context, player, buffer, "Jumpspeed", &value))
    return;
  value *= KPH_PER_MP;
  if (value < 0.0F || !isfinite(value)) {
    notify_printf(btech_context_evaluation(context), player,
                  "Invalid value for SetJumpspeed!");
    return;
  }
  btech_admin_jump_speed_set(mech, value / KPH_PER_MP);
  notify_repair_float(context, player, "Jumpspeed", mech_jump_speed(mech));
}

void mechrep_rsetheatsinks(DbRef player, void *data, char *buffer) {
  int value;
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (!parse_repair_int(context, player, buffer, "Heatsinks", &value))
    return;
  if (!validate_repair_int_range(context, player, "Heatsinks", value, 0,
                                 CHAR_MAX))
    return;
  btech_admin_heat_sinks_set(mech, value);
  notify_repair_int(context, player, "Heatsinks", mech_heat_sink_count(mech));
}

void mechrep_rsetlrsrange(DbRef player, void *data, char *buffer) {
  int value;
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (!parse_repair_int(context, player, buffer, "LRSrange", &value))
    return;
  if (!validate_repair_int_range(context, player, "LRSrange", value, 0,
                                 CHAR_MAX))
    return;
  btech_admin_long_range_set(mech, value);
  notify_repair_int(context, player, "LRSrange",
                    mech_long_range_sensor_range(mech));
}

void mechrep_rsettacrange(DbRef player, void *data, char *buffer) {
  int value;
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (!parse_repair_int(context, player, buffer, "TACrange", &value))
    return;
  if (!validate_repair_int_range(context, player, "TACrange", value, 0,
                                 CHAR_MAX))
    return;
  btech_admin_tactical_range_set(mech, value);
  notify_repair_int(context, player, "TACrange", mech_tactical_range(mech));
}

void mechrep_rsetscanrange(DbRef player, void *data, char *buffer) {
  int value;
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (!parse_repair_int(context, player, buffer, "SCANrange", &value))
    return;
  if (!validate_repair_int_range(context, player, "SCANrange", value, 0,
                                 CHAR_MAX))
    return;
  btech_admin_scan_range_set(mech, value);
  notify_repair_int(context, player, "SCANrange", mech_scanner_range(mech));
}

void mechrep_rsetradiorange(DbRef player, void *data, char *buffer) {
  int value;
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (!parse_repair_int(context, player, buffer, "RADIOrange", &value))
    return;
  if (!validate_repair_int_range(context, player, "RADIOrange", value, 0,
                                 SHRT_MAX))
    return;
  btech_admin_radio_range_set(mech, value);
  notify_repair_int(context, player, "RADIOrange", mech_radio_range(mech));
}

void mechrep_rsettons(DbRef player, void *data, char *buffer) {
  int value;
  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (!parse_repair_int(context, player, buffer, "Tons", &value))
    return;
  if (!validate_repair_int_range(context, player, "Tons", value, 1, INT_MAX))
    return;
  mech_tonnage_set(mech, value);
  notify_repair_int(context, player, "Tons", mech_tonnage(mech));
}

void mechrep_rsetmove(DbRef player, void *data, char *buffer) {
  char *args[1];

  MechAdminCommandContext repair_command;
  RepairCommandStatus repair_status =
      mech_admin_command_context_initialize(player, data, &repair_command);
  if (repair_status != REPAIR_COMMAND_READY) {
    if (repair_command.evaluation)
      mecha_notify(repair_command.evaluation, player,
                   repair_command_status_message(repair_status));
    return;
  }
  BtechContext *context = repair_command.context;
  Mech *mech = repair_command.mech;
  if (mech_parseattributes(buffer, args, 1) != 1) {
    mecha_notify(btech_context_evaluation(context), player,
                 "Invalid number of arguments!");
    return;
  }
  char *movement =
      *(char **)checked_storage_at((void *)args, 1, sizeof(*args), 0);
  switch (ascii_to_upper(*movement)) {
  case 'T':
    btech_admin_movement_type_set(mech, MOVE_TRACK);
    mecha_notify(btech_context_evaluation(context), player,
                 "Movement set to TRACKED");
    break;
  case 'W':
    btech_admin_movement_type_set(mech, MOVE_WHEEL);
    mecha_notify(btech_context_evaluation(context), player,
                 "Movement set to WHEELED");
    break;
  case 'H':
    switch (ascii_to_upper(*checked_string_suffix(movement, 1))) {
    case 'O':
      btech_admin_movement_type_set(mech, MOVE_HOVER);
      mecha_notify(btech_context_evaluation(context), player,
                   "Movement set to HOVER");
      break;
    case 'U':
      btech_admin_movement_type_set(mech, MOVE_HULL);
      mecha_notify(btech_context_evaluation(context), player,
                   "Movement set to HULL");
      break;
    }
    break;
  case 'V':
    btech_admin_movement_type_set(mech, MOVE_VTOL);
    mecha_notify(btech_context_evaluation(context), player,
                 "Movement set to VTOL");
    break;
  case 'Q':
    btech_admin_movement_type_set(mech, MOVE_QUAD);
    mecha_notify(btech_context_evaluation(context), player,
                 "Movement set to QUAD");
    break;
  case 'B':
    btech_admin_movement_type_set(mech, MOVE_BIPED);
    mecha_notify(btech_context_evaluation(context), player,
                 "Movement set to BIPED");
    break;
  case 'S':
    btech_admin_movement_type_set(mech, MOVE_SUB);
    mecha_notify(btech_context_evaluation(context), player,
                 "Movement set to SUB");
    break;
  case 'F':
    switch (ascii_to_upper(*checked_string_suffix(movement, 1))) {
    case 'O':
      btech_admin_movement_type_set(mech, MOVE_FOIL);
      mecha_notify(btech_context_evaluation(context), player,
                   "Movement set to FOIL");
      break;
    case 'L':
      btech_admin_movement_type_set(mech, MOVE_FLY);
      mecha_notify(btech_context_evaluation(context), player,
                   "Movement set to FLY");
      break;
    }
    break;
  case 'N':
    btech_admin_movement_type_set(mech, MOVE_NONE);
    mecha_notify(btech_context_evaluation(context), player,
                 "Movement set to NONE");
    break;
  default:
    mecha_notify(btech_context_evaluation(context), player,
                 "Types are: TRACK, WHEEL, VTOL, QUAD, BIPED, HOVER, HULL, "
                 "FLY, SUB, FOIL and NONE");
    break;
  }
}
