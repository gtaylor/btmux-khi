#include <stdio.h>
#include <string.h>

#include "autopilot.h"
#include "autopilot_argument_list_api.h"
#include "autopilot_commands_api.h"
#include "autopilot_control_api.h"
#include "btech/autopilot.h"
#include "btech/context.h"
#include "context_internal.h" // IWYU pragma: keep
#include "mech_utils_api.h"
#include "mux/network/network_output.h"
#include "mux/server/platform.h"
#include "mux/support/alloc.h"
#include "mux/support/doubly_linked_list.h"
#include "mux/support/stringutil.h"
#include "registry_api.h"

static BtechAutopilotResult remove_result;
static BtechAutopilotResult clear_result;
static BtechAutopilotResult engage_result;
static int engage_calls;
static int gun_calls;
static int profile_calls;
static int sensor_calls;
static DbRef target_result;
static char notification[160];

EvaluationContext *btech_context_evaluation(BtechContext *context
                                            [[maybe_unused]]) {
  return nullptr;
}

void mecha_notify(EvaluationContext *evaluation [[maybe_unused]],
                  DbRef player [[maybe_unused]], const char *msg) {
  (void)snprintf(notification, sizeof(notification), "%s", msg);
}

void notify_printf(EvaluationContext *evaluation [[maybe_unused]],
                   DbRef player [[maybe_unused]], const char *format, ...) {
  (void)snprintf(notification, sizeof(notification), "%s", format);
}

bool parse_int_checked(const char *text, int *value) {
  if (strcmp(text, "1") == 0) {
    *value = 1;
    return true;
  }
  if (strcmp(text, "-1") == 0) {
    *value = -1;
    return true;
  }
  return false;
}

BtechAutopilotResult autopilot_control_remove(Autopilot *autopilot
                                              [[maybe_unused]],
                                              size_t position
                                              [[maybe_unused]]) {
  return remove_result;
}

BtechAutopilotResult autopilot_control_clear(Autopilot *autopilot
                                             [[maybe_unused]]) {
  return clear_result;
}

BtechAutopilotResult autopilot_control_engage(Autopilot *autopilot) {
  engage_calls++;
  if (engage_result == BTECH_AUTOPILOT_OK)
    autopilot->engaged = true;
  return engage_result;
}

DbRef find_target_dbref_from_map_number(Mech *mech [[maybe_unused]],
                                        const char *mapnum [[maybe_unused]]) {
  return target_result;
}

void auto_sensor_event(Autopilot *autopilot [[maybe_unused]]) {
  sensor_calls++;
}

void auto_update_profile_event(Autopilot *autopilot [[maybe_unused]]) {
  profile_calls++;
}

void auto_gun_event(Autopilot *autopilot [[maybe_unused]]) { gun_calls++; }

static bool reports(const char *fragment) {
  return strstr(notification, fragment) != nullptr;
}

static void radio_arguments(AutopilotArgumentList *arguments,
                            const char *command, const char *value) {
  autopilot_argument_list_initialize(arguments, 2);
  autopilot_argument_list_set(arguments, 0, strdup(command));
  autopilot_argument_list_set(arguments, 1, strdup(value));
}

static int test_radio_autogun_reengages(Autopilot *autopilot, Mech *mech) {
  char message[LBUF_SIZE] = {0};
  AutopilotArgumentList arguments;
  engage_calls = 0;
  gun_calls = 0;
  profile_calls = 0;
  sensor_calls = 0;
  engage_result = BTECH_AUTOPILOT_OK;
  autopilot->engaged = false;
  autopilot->mymech = mech;
  autopilot->target = 91;
  autopilot->flags = AUTOPILOT_ASSIGNED_TARGET | AUTOPILOT_GUNZOMBIE;

  radio_arguments(&arguments, "autogun", "on");
  auto_radio_command_autogun(autopilot, mech, &arguments, 2, message);
  autopilot_argument_list_destroy(&arguments);
  if (!autopilot->engaged || engage_calls != 1 ||
      !autopilot_is_gunning(autopilot) ||
      autopilot_has_assigned_target(autopilot) || autopilot->target != -1 ||
      autopilot->target_update_tick != AUTO_GUN_UPDATE_TICK)
    return 1;

  auto_heartbeat(autopilot);
  if (sensor_calls != 1 || profile_calls != 1 || gun_calls != 1)
    return 2;

  engage_result = BTECH_AUTOPILOT_ALREADY_ENGAGED;
  radio_arguments(&arguments, "target", "-");
  auto_radio_command_target(autopilot, mech, &arguments, 2, message);
  autopilot_argument_list_destroy(&arguments);
  if (!autopilot->engaged || engage_calls != 2 ||
      !autopilot_is_gunning(autopilot) || autopilot->target != -1)
    return 3;

  autopilot->engaged = false;
  engage_result = BTECH_AUTOPILOT_OK;
  target_result = 44;
  radio_arguments(&arguments, "target", "AA");
  auto_radio_command_target(autopilot, mech, &arguments, 2, message);
  autopilot_argument_list_destroy(&arguments);
  if (!autopilot->engaged || engage_calls != 3 ||
      !autopilot_is_gunning(autopilot) ||
      !autopilot_has_assigned_target(autopilot) || autopilot->target != 44 ||
      autopilot->target_update_tick != 0)
    return 4;
  return 0;
}

static int test_radio_autogun_failures_are_atomic(Autopilot *autopilot,
                                                  Mech *mech) {
  char message[LBUF_SIZE] = {0};
  AutopilotArgumentList arguments;
  engage_calls = 0;
  engage_result = BTECH_AUTOPILOT_CONFLICT;
  autopilot->engaged = false;
  autopilot->target = 73;
  autopilot->target_score = 19;
  autopilot->target_update_tick = 7;
  autopilot->flags = AUTOPILOT_ASSIGNED_TARGET | AUTOPILOT_GUNZOMBIE;

  radio_arguments(&arguments, "autogun", "on");
  auto_radio_command_autogun(autopilot, mech, &arguments, 2, message);
  autopilot_argument_list_destroy(&arguments);
  if (autopilot->engaged || engage_calls != 1 || autopilot->target != 73 ||
      autopilot->target_score != 19 || autopilot->target_update_tick != 7 ||
      autopilot->flags != (AUTOPILOT_ASSIGNED_TARGET | AUTOPILOT_GUNZOMBIE) ||
      strstr(message, "association conflict") == nullptr)
    return 1;

  engage_result = BTECH_AUTOPILOT_NOT_INSTALLED_IN_UNIT;
  memset(message, 0, sizeof(message));
  radio_arguments(&arguments, "target", "-");
  auto_radio_command_target(autopilot, mech, &arguments, 2, message);
  autopilot_argument_list_destroy(&arguments);
  if (autopilot->engaged || engage_calls != 2 || autopilot->target != 73 ||
      autopilot->target_score != 19 || autopilot->target_update_tick != 7 ||
      autopilot->flags != (AUTOPILOT_ASSIGNED_TARGET | AUTOPILOT_GUNZOMBIE) ||
      strcmp(message, "!Unable to engage autopilot: not installed in a unit") !=
          0)
    return 2;

  target_result = -1;
  memset(message, 0, sizeof(message));
  radio_arguments(&arguments, "target", "XX");
  auto_radio_command_target(autopilot, mech, &arguments, 2, message);
  autopilot_argument_list_destroy(&arguments);
  if (engage_calls != 2 || autopilot->target != 73 ||
      strstr(message, "Unable to see") == nullptr)
    return 3;

  radio_arguments(&arguments, "autogun", "off");
  auto_radio_command_autogun(autopilot, mech, &arguments, 2, message);
  autopilot_argument_list_destroy(&arguments);
  if (engage_calls != 2 || autopilot->target != -2 ||
      autopilot_is_gunning(autopilot) ||
      autopilot_has_assigned_target(autopilot))
    return 4;
  return 0;
}

int main(void) {
  BtechContext context = {.tick = 1};
  Mech *const MECH = (Mech *)2;
  Autopilot autopilot = {
      .xcode.context = &context,
      .commands = doubly_linked_list_create_list(),
  };
  DoublyLinkedListNode *node = doubly_linked_list_create_node(&autopilot);
  doubly_linked_list_insert_end(autopilot.commands, node);

  remove_result = BTECH_AUTOPILOT_ACTIVE_ORDER;
  auto_delcommand(1, &autopilot, "1");
  if (!reports("Disengage") || reports("Successfully"))
    return 1;
  remove_result = BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  auto_delcommand(1, &autopilot, "1");
  if (!reports("Unable to remove") || reports("Successfully"))
    return 2;
  remove_result = BTECH_AUTOPILOT_OK;
  auto_delcommand(1, &autopilot, "1");
  if (!reports("Successfully Removed"))
    return 3;

  clear_result = BTECH_AUTOPILOT_ACTIVE_ORDER;
  auto_delcommand(1, &autopilot, "-1");
  if (!reports("Disengage") || reports("commands have been removed"))
    return 4;
  clear_result = BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  auto_delcommand(1, &autopilot, "-1");
  if (!reports("Unable to clear") || reports("commands have been removed"))
    return 5;
  clear_result = BTECH_AUTOPILOT_OK;
  auto_delcommand(1, &autopilot, "-1");
  if (!reports("commands have been removed"))
    return 6;

  engage_result = BTECH_AUTOPILOT_CONFLICT;
  auto_engage(1, &autopilot, "");
  if (!reports("already associated elsewhere"))
    return 7;

  const int RADIO_REENGAGE = test_radio_autogun_reengages(&autopilot, MECH);
  if (RADIO_REENGAGE != 0)
    return 10 + RADIO_REENGAGE;
  const int RADIO_FAILURES =
      test_radio_autogun_failures_are_atomic(&autopilot, MECH);
  if (RADIO_FAILURES != 0)
    return 20 + RADIO_FAILURES;

  (void)doubly_linked_list_remove_node_at_pos(autopilot.commands, 1);
  doubly_linked_list_destroy_list(autopilot.commands);
  return 0;
}
