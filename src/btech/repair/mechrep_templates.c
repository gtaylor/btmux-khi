/* Implements BattleTech repair mechanics for mechrep templates. */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "btech/context.h"
#include "btech_event.h"
#include "command_handlers_api.h"
#include "map_terrain.h"    // IWYU pragma: keep
#include "mech_lifecycle.h" // IWYU pragma: keep
#include "mux/objects/flags.h"
#include "mux/server/game.h"
#include "mux/server/platform.h"
#include "registry_api.h"
#include "repair_job.h"

#include "mech_consistency_api.h"
#include "mech_identity_api.h"
#include "mech_restrict_api.h"
#include "mech_specification_api.h"
#include "mech_template_api.h"
#include "mech_utils_api.h"
#include "mechrep_api.h"
#include "template_api.h"

bool btech_admin_template_load(DbRef player, Mech *mech,
                               const char *reference) {
  if (!mech_template_load(player, mech, reference))
    return false;
  mech_events_cancel_all(mech);
  clear_mech_from_los(mech);
  return true;
}

bool btech_admin_template_restore(DbRef player, Mech *mech) {
  const char *reference = mech_model_reference(mech);
  if (reference == nullptr || *reference == '\0')
    return false;
  return btech_admin_template_load(player, mech, reference);
}

bool btech_admin_template_save(DbRef player, Mech *mech,
                               const char *reference) {
  char filename[512] = {0};
  mech_template_registry_clear(mech_context(mech));
  const int WRITTEN =
      snprintf(filename, sizeof(filename), "%s/",
               btech_context_mech_template_path(mech_context(mech)));
  if (WRITTEN < 0 || (size_t)WRITTEN >= sizeof(filename) ||
      !string_append_bounded(filename, sizeof(filename), reference))
    return false;
  return template_save(&(TemplateSaveRequest){
             .player = player,
             .mech = mech,
             .reference = reference,
             .filename = filename,
         }) >= 0;
}

/* Selectors */

/*--------------------------------------------------------------------------*/

/* Code Begins                                                              */

/*--------------------------------------------------------------------------*/

/* Alloc free function */

/* Alloc/free routine */

void mechrep_rloadnew(DbRef player, void *data, char *buffer) {
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
  if (mech_parseattributes(buffer, args, 1) == 1) {
    if (btech_admin_template_load(player, mech, args[0])) {
      mecha_notify(btech_context_evaluation(context), player,
                   "Template loaded.");
      return;
    }
  }
  mecha_notify(btech_context_evaluation(context), player,
               "Unable to read that template.");
}

void mechrep_rrestore(DbRef player, void *data, char *buffer [[maybe_unused]]) {
  const char *c;

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
  c = mech_model_reference(mech);
  if (!c || !*c) {
    mecha_notify(btech_context_evaluation(context), player,
                 "Sorry, I don't know what type of mech this is");
    return;
  }
  if (btech_admin_template_restore(player, mech)) {
    mecha_notify(btech_context_evaluation(context), player,
                 "Restoration complete!");
    return;
  }
  mecha_notify(btech_context_evaluation(context), player,
               "Unable to restore this mech!.");
}

/*
 * Template saving routines and logic.
 */
void mechrep_rsavetemp2(DbRef player, void *data, char *buffer) {
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

  // No template name given.
  if (mech_parseattributes(buffer, args, 1) != 1) {
    mecha_notify(btech_context_evaluation(context), player,
                 "You must specify a template name!");
    return;
  }

  // Anti-twink measure. Don't allow directory saving... yet
  if (strstr(args[0], "/")) {
    mecha_notify(btech_context_evaluation(context), player,
                 "Invalid file name!");
    return;
  }

  notify_printf(btech_context_evaluation(context), player, "Saving %s",
                args[0]);
  // Just warn on overweight.
  if (mech_weight_sub(GOD, mech, -1) > (mech_tonnage(mech) * 1024))
    mecha_notify(btech_context_evaluation(context), player,
                 "Warning: Template Overweight, see @weight.");

  // I/O or Permissions error.
  if (!btech_admin_template_save(player, mech, args[0])) {
    mecha_notify(btech_context_evaluation(context), player,
                 "Error saving the template file!");
    return;
  }

  mecha_notify(btech_context_evaluation(context), player, "Saving complete!");
} // end mechrep_Rsavetemp2

/*
 * Emits the valid sections when a player tries to setarmor/addsp/reload an
 * invalid section
 */
