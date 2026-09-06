#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autopilot_argument_list_api.h"
#include "autopilot_order_codec_api.h"
#include "mux/support/checked_storage.h"
#include "registry_api.h"

static const AutopilotCommandDefinition DEFINITIONS[AUTO_NUM_COMMANDS + 1] = {
    [GOAL_CHASETARGET] = {"chasetarget", 1, GOAL_CHASETARGET, nullptr},
    [GOAL_DUMBFOLLOW] = {"dumbfollow", 1, GOAL_DUMBFOLLOW, nullptr},
    [GOAL_DUMBGOTO] = {"dumbgoto", 2, GOAL_DUMBGOTO, nullptr},
    [GOAL_ENTERBASE] = {"enterbase", 1, GOAL_ENTERBASE, nullptr},
    [GOAL_FOLLOW] = {"follow", 1, GOAL_FOLLOW, nullptr},
    [GOAL_GOTO] = {"goto", 2, GOAL_GOTO, nullptr},
    [GOAL_LEAVEBASE] = {"leavebase", 1, GOAL_LEAVEBASE, nullptr},
    [GOAL_OLDGOTO] = {"oldgoto", 2, GOAL_OLDGOTO, nullptr},
    [GOAL_ROAM] = {"roam", 1, GOAL_ROAM, nullptr},
    [GOAL_WAIT] = {"wait", 2, GOAL_WAIT, nullptr},
    [COMMAND_ATTACKLEG] = {"attackleg", 1, COMMAND_ATTACKLEG, nullptr},
    [COMMAND_AUTOGUN] = {"autogun", 1, COMMAND_AUTOGUN, nullptr},
    [COMMAND_CHASEMODE] = {"chasemode", 1, COMMAND_CHASEMODE, nullptr},
    [COMMAND_CMODE] = {"cmode", 2, COMMAND_CMODE, nullptr},
    [COMMAND_DROPOFF] = {"dropoff", 0, COMMAND_DROPOFF, nullptr},
    [COMMAND_EMBARK] = {"embark", 1, COMMAND_EMBARK, nullptr},
    [COMMAND_ENTERBAY] = {"enterbay", 0, COMMAND_ENTERBAY, nullptr},
    [COMMAND_JUMP] = {"jump", 1, COMMAND_JUMP, nullptr},
    [COMMAND_LOAD] = {"load", 0, COMMAND_LOAD, nullptr},
    [COMMAND_PICKUP] = {"pickup", 1, COMMAND_PICKUP, nullptr},
    [COMMAND_REPORT] = {"report", 0, COMMAND_REPORT, nullptr},
    [COMMAND_ROAMMODE] = {"roammode", 1, COMMAND_ROAMMODE, nullptr},
    [COMMAND_SHUTDOWN] = {"shutdown", 0, COMMAND_SHUTDOWN, nullptr},
    [COMMAND_SPEED] = {"speed", 1, COMMAND_SPEED, nullptr},
    [COMMAND_STARTUP] = {"startup", 0, COMMAND_STARTUP, nullptr},
    [COMMAND_STOPGUN] = {"stopgun", 0, COMMAND_STOPGUN, nullptr},
    [COMMAND_SWARM] = {"swarm", 1, COMMAND_SWARM, nullptr},
    [COMMAND_SWARMMODE] = {"swarmmode", 1, COMMAND_SWARMMODE, nullptr},
    [COMMAND_UDISEMBARK] = {"udisembark", 0, COMMAND_UDISEMBARK, nullptr},
    [COMMAND_UNLOAD] = {"unload", 0, COMMAND_UNLOAD, nullptr},
    [AUTO_NUM_COMMANDS] = {nullptr, 0, AUTO_NUM_COMMANDS, nullptr},
};

const AutopilotCommandDefinition *autopilot_command_definition_at(int index) {
  return index >= 0 && index <= AUTO_NUM_COMMANDS
             ? checked_storage_at_const(DEFINITIONS, AUTO_NUM_COMMANDS + 1,
                                        sizeof(*DEFINITIONS), (size_t)index)
             : nullptr;
}

bool btech_context_is_mech(BtechContext *context, DbRef key) {
  return context != nullptr && key == 42;
}

Mech *btech_context_get_mech(BtechContext *context, DbRef key) {
  return btech_context_is_mech(context, key) ? (Mech *)context : nullptr;
}

static bool same_order(const BtechAutopilotOrder *left,
                       const BtechAutopilotOrder *right) {
  if (left->type != right->type)
    return false;
  switch (left->type) {
  case BTECH_AUTOPILOT_ORDER_CHASETARGET:
  case BTECH_AUTOPILOT_ORDER_DUMBFOLLOW:
  case BTECH_AUTOPILOT_ORDER_FOLLOW:
  case BTECH_AUTOPILOT_ORDER_EMBARK:
  case BTECH_AUTOPILOT_ORDER_PICKUP:
    return left->data.target.target == right->data.target.target;
  case BTECH_AUTOPILOT_ORDER_DUMBGOTO:
  case BTECH_AUTOPILOT_ORDER_GOTO:
  case BTECH_AUTOPILOT_ORDER_OLDGOTO:
    return left->data.destination.x == right->data.destination.x &&
           left->data.destination.y == right->data.destination.y;
  case BTECH_AUTOPILOT_ORDER_ENTERBASE:
    return left->data.enter_base.direction == right->data.enter_base.direction;
  case BTECH_AUTOPILOT_ORDER_LEAVEBASE:
    return left->data.leave_base.heading == right->data.leave_base.heading;
  case BTECH_AUTOPILOT_ORDER_ROAM:
    return left->data.roam.mode == right->data.roam.mode &&
           (left->data.roam.mode == BTECH_AUTOPILOT_ROAM_MAP ||
            (left->data.roam.x == right->data.roam.x &&
             left->data.roam.y == right->data.roam.y &&
             left->data.roam.radius == right->data.roam.radius));
  case BTECH_AUTOPILOT_ORDER_AUTOGUN:
    return left->data.autogun.mode == right->data.autogun.mode &&
           (left->data.autogun.mode != BTECH_AUTOPILOT_AUTOGUN_TARGET ||
            left->data.autogun.target == right->data.autogun.target);
  case BTECH_AUTOPILOT_ORDER_SPEED:
    return left->data.speed.percent == right->data.speed.percent;
  case BTECH_AUTOPILOT_ORDER_DROPOFF:
  case BTECH_AUTOPILOT_ORDER_SHUTDOWN:
  case BTECH_AUTOPILOT_ORDER_STARTUP:
  case BTECH_AUTOPILOT_ORDER_UDISEMBARK:
    return true;
  }
  return false;
}

static int round_trip(BtechContext *context, const BtechAutopilotOrder *input,
                      const char *name, const char *first, const char *second) {
  const AutopilotCommandDefinition *definition = nullptr;
  AutopilotArgumentList arguments = {};
  BtechAutopilotOrder output = {};

  if (autopilot_order_encode(context, input, &definition, &arguments) !=
          BTECH_AUTOPILOT_OK ||
      definition == nullptr || strcmp(definition->name, name) != 0 ||
      strcmp(autopilot_argument_list_get(&arguments, 0), name) != 0 ||
      ((first == nullptr) !=
       (autopilot_argument_list_get(&arguments, 1) == nullptr)) ||
      (first != nullptr &&
       strcmp(autopilot_argument_list_get(&arguments, 1), first) != 0) ||
      ((second == nullptr) !=
       (autopilot_argument_list_get(&arguments, 2) == nullptr)) ||
      (second != nullptr &&
       strcmp(autopilot_argument_list_get(&arguments, 2), second) != 0)) {
    autopilot_argument_list_destroy(&arguments);
    return -1;
  }
  AutopilotCommand command = {.arguments = arguments,
                              .argcount = (unsigned char)definition->argcount,
                              .command_enum = definition->command_enum};
  const bool matched =
      autopilot_order_decode(&command, &output) == BTECH_AUTOPILOT_OK &&
      same_order(input, &output);
  autopilot_argument_list_destroy(&arguments);
  return matched ? 0 : -1;
}

static int test_round_trips(BtechContext *context) {
  static const BtechAutopilotOrder ORDERS[] = {
      {.type = BTECH_AUTOPILOT_ORDER_CHASETARGET, .data.target.target = 42},
      {.type = BTECH_AUTOPILOT_ORDER_DUMBFOLLOW, .data.target.target = 42},
      {.type = BTECH_AUTOPILOT_ORDER_DUMBGOTO,
       .data.destination = {.x = 0, .y = 2147483647}},
      {.type = BTECH_AUTOPILOT_ORDER_ENTERBASE,
       .data.enter_base.direction = BTECH_AUTOPILOT_WEST},
      {.type = BTECH_AUTOPILOT_ORDER_FOLLOW, .data.target.target = 42},
      {.type = BTECH_AUTOPILOT_ORDER_GOTO,
       .data.destination = {.x = 12, .y = 34}},
      {.type = BTECH_AUTOPILOT_ORDER_LEAVEBASE, .data.leave_base.heading = 359},
      {.type = BTECH_AUTOPILOT_ORDER_OLDGOTO,
       .data.destination = {.x = 56, .y = 78}},
      {.type = BTECH_AUTOPILOT_ORDER_ROAM,
       .data.roam = {.mode = BTECH_AUTOPILOT_ROAM_MAP}},
      {.type = BTECH_AUTOPILOT_ORDER_ROAM,
       .data.roam = {.mode = BTECH_AUTOPILOT_ROAM_RADIUS,
                     .x = 9,
                     .y = 10,
                     .radius = 30}},
      {.type = BTECH_AUTOPILOT_ORDER_AUTOGUN,
       .data.autogun.mode = BTECH_AUTOPILOT_AUTOGUN_AUTOMATIC},
      {.type = BTECH_AUTOPILOT_ORDER_AUTOGUN,
       .data.autogun.mode = BTECH_AUTOPILOT_AUTOGUN_OFF},
      {.type = BTECH_AUTOPILOT_ORDER_AUTOGUN,
       .data.autogun = {.mode = BTECH_AUTOPILOT_AUTOGUN_TARGET, .target = 42}},
      {.type = BTECH_AUTOPILOT_ORDER_DROPOFF},
      {.type = BTECH_AUTOPILOT_ORDER_EMBARK, .data.target.target = 42},
      {.type = BTECH_AUTOPILOT_ORDER_PICKUP, .data.target.target = 42},
      {.type = BTECH_AUTOPILOT_ORDER_SHUTDOWN},
      {.type = BTECH_AUTOPILOT_ORDER_SPEED, .data.speed.percent = 100},
      {.type = BTECH_AUTOPILOT_ORDER_STARTUP},
      {.type = BTECH_AUTOPILOT_ORDER_UDISEMBARK},
  };
  static const char *const NAMES[] = {
      "chasetarget", "dumbfollow", "dumbgoto", "enterbase", "follow",
      "goto",        "leavebase",  "oldgoto",  "roam",      "roam",
      "autogun",     "autogun",    "autogun",  "dropoff",   "embark",
      "pickup",      "shutdown",   "speed",    "startup",   "udisembark",
  };
  static const char *const FIRST[] = {
      "42", "42",    "0",         "west",  "42",
      "12", "359",   "56",        "map",   "radius 9 10 30",
      "on", "off",   "target 42", nullptr, "42",
      "42", nullptr, "100",       nullptr, nullptr,
  };
  static const char *const SECOND[] = {
      nullptr, nullptr, "2147483647", nullptr, nullptr, "34",    nullptr,
      "78",    nullptr, nullptr,      nullptr, nullptr, nullptr, nullptr,
      nullptr, nullptr, nullptr,      nullptr, nullptr, nullptr,
  };

  for (size_t index = 0; index < sizeof(ORDERS) / sizeof(*ORDERS); index++)
    if (round_trip(
            context,
            checked_storage_at_const(ORDERS, sizeof(ORDERS) / sizeof(*ORDERS),
                                     sizeof(*ORDERS), index),
            *(const char *const *)checked_storage_at_const(
                NAMES, sizeof(NAMES) / sizeof(*NAMES), sizeof(*NAMES), index),
            *(const char *const *)checked_storage_at_const(
                FIRST, sizeof(FIRST) / sizeof(*FIRST), sizeof(*FIRST), index),
            *(const char *const *)checked_storage_at_const(
                SECOND, sizeof(SECOND) / sizeof(*SECOND), sizeof(*SECOND),
                index)) < 0)
      return (int)index + 1;
  return 0;
}

static int test_validation(BtechContext *context) {
  BtechAutopilotOrder value = {.type = BTECH_AUTOPILOT_ORDER_SPEED,
                               .data.speed.percent = 0};
  if (btech_autopilot_order_validate(context, &value) !=
      BTECH_AUTOPILOT_INVALID_ARGUMENT)
    return 1;
  value.data.speed.percent = 101;
  if (btech_autopilot_order_validate(context, &value) !=
      BTECH_AUTOPILOT_INVALID_ARGUMENT)
    return 2;
  value = (BtechAutopilotOrder){.type = BTECH_AUTOPILOT_ORDER_LEAVEBASE,
                                .data.leave_base.heading = 360};
  if (btech_autopilot_order_validate(context, &value) !=
      BTECH_AUTOPILOT_INVALID_ARGUMENT)
    return 3;
  value = (BtechAutopilotOrder){
      .type = BTECH_AUTOPILOT_ORDER_ROAM,
      .data.roam = {.mode = BTECH_AUTOPILOT_ROAM_RADIUS, .radius = 31}};
  if (btech_autopilot_order_validate(context, &value) !=
      BTECH_AUTOPILOT_INVALID_ARGUMENT)
    return 4;
  value = (BtechAutopilotOrder){.type = BTECH_AUTOPILOT_ORDER_GOTO,
                                .data.destination = {.x = -1, .y = 0}};
  if (btech_autopilot_order_validate(context, &value) !=
      BTECH_AUTOPILOT_INVALID_ARGUMENT)
    return 5;
  value = (BtechAutopilotOrder){.type = BTECH_AUTOPILOT_ORDER_FOLLOW,
                                .data.target.target = 41};
  if (btech_autopilot_order_validate(context, &value) !=
      BTECH_AUTOPILOT_UNAVAILABLE)
    return 6;
  value.type = (BtechAutopilotOrderType)999;
  if (btech_autopilot_order_validate(context, &value) !=
          BTECH_AUTOPILOT_INVALID_ARGUMENT ||
      btech_autopilot_order_validate(nullptr, &value) !=
          BTECH_AUTOPILOT_INVALID_ARGUMENT ||
      btech_autopilot_order_validate(context, nullptr) !=
          BTECH_AUTOPILOT_INVALID_ARGUMENT)
    return 7;
  return 0;
}

static int test_enterbase_legacy_decode(BtechContext *context) {
  typedef struct EnterbaseDirectionCase {
    const char *token;
    BtechAutopilotDirection direction;
    const char *canonical;
  } EnterbaseDirectionCase;
  static const EnterbaseDirectionCase VALID[] = {
      {"north", BTECH_AUTOPILOT_NORTH, "north"},
      {"east", BTECH_AUTOPILOT_EAST, "east"},
      {"south", BTECH_AUTOPILOT_SOUTH, "south"},
      {"west", BTECH_AUTOPILOT_WEST, "west"},
      {"n", BTECH_AUTOPILOT_NORTH, "north"},
      {"N", BTECH_AUTOPILOT_NORTH, "north"},
      {"e", BTECH_AUTOPILOT_EAST, "east"},
      {"E", BTECH_AUTOPILOT_EAST, "east"},
      {"s", BTECH_AUTOPILOT_SOUTH, "south"},
      {"S", BTECH_AUTOPILOT_SOUTH, "south"},
      {"w", BTECH_AUTOPILOT_WEST, "west"},
      {"W", BTECH_AUTOPILOT_WEST, "west"},
      {"NoRtH", BTECH_AUTOPILOT_NORTH, "north"},
      {"eastbound", BTECH_AUTOPILOT_EAST, "east"},
      {"South", BTECH_AUTOPILOT_SOUTH, "south"},
      {"WESTWARD", BTECH_AUTOPILOT_WEST, "west"},
      {"northwest", BTECH_AUTOPILOT_NORTH, "north"},
  };
  static const char *const INVALID[] = {"", "x", "orth", "1north"};

  for (size_t index = 0; index < sizeof(VALID) / sizeof(*VALID); index++) {
    const EnterbaseDirectionCase *test_case = checked_storage_at_const(
        VALID, sizeof(VALID) / sizeof(*VALID), sizeof(*VALID), index);
    AutopilotCommand command = {.argcount = 1, .command_enum = GOAL_ENTERBASE};
    autopilot_argument_list_initialize(&command.arguments, AUTOPILOT_MAX_ARGS);
    autopilot_argument_list_set(&command.arguments, 0, strdup("enterbase"));
    autopilot_argument_list_set(&command.arguments, 1,
                                strdup(test_case->token));
    BtechAutopilotOrder order = {};
    const bool decoded =
        autopilot_order_decode(&command, &order) == BTECH_AUTOPILOT_OK &&
        order.type == BTECH_AUTOPILOT_ORDER_ENTERBASE &&
        order.data.enter_base.direction == test_case->direction;
    autopilot_argument_list_destroy(&command.arguments);
    if (!decoded)
      return (int)index + 1;

    const AutopilotCommandDefinition *definition = nullptr;
    AutopilotArgumentList encoded = {};
    const bool canonical =
        autopilot_order_encode(context, &order, &definition, &encoded) ==
            BTECH_AUTOPILOT_OK &&
        definition != nullptr && definition->command_enum == GOAL_ENTERBASE &&
        strcmp(autopilot_argument_list_get(&encoded, 1),
               test_case->canonical) == 0;
    autopilot_argument_list_destroy(&encoded);
    if (!canonical)
      return (int)index + 20;
  }

  for (size_t index = 0; index < sizeof(INVALID) / sizeof(*INVALID); index++) {
    const char *token = *(const char *const *)checked_storage_at_const(
        INVALID, sizeof(INVALID) / sizeof(*INVALID), sizeof(*INVALID), index);
    AutopilotCommand command = {.argcount = 1, .command_enum = GOAL_ENTERBASE};
    autopilot_argument_list_initialize(&command.arguments, AUTOPILOT_MAX_ARGS);
    autopilot_argument_list_set(&command.arguments, 0, strdup("enterbase"));
    autopilot_argument_list_set(&command.arguments, 1, strdup(token));
    BtechAutopilotOrder order = {};
    const BtechAutopilotResult result =
        autopilot_order_decode(&command, &order);
    autopilot_argument_list_destroy(&command.arguments);
    if (result != BTECH_AUTOPILOT_INVALID_ORDER_QUEUE)
      return (int)index + 40;
  }
  return 0;
}

static int test_numeric_decode(void) {
  char long_max[32];
  (void)snprintf(long_max, sizeof(long_max), "%ld", LONG_MAX);
  static const char *const INVALID[] = {
      "",
      "+50",
      "050",
      "-0",
      " 50",
      "50 ",
      "50x",
      "999999999999999999999999",
      "-999999999999999999999999",
  };
  AutopilotCommand command = {.argcount = 1, .command_enum = GOAL_FOLLOW};
  autopilot_argument_list_initialize(&command.arguments, AUTOPILOT_MAX_ARGS);
  autopilot_argument_list_set(&command.arguments, 0, strdup("follow"));
  autopilot_argument_list_set(&command.arguments, 1, strdup(long_max));
  BtechAutopilotOrder output;
  bool valid =
      autopilot_order_decode(&command, &output) == BTECH_AUTOPILOT_OK &&
      output.data.target.target == LONG_MAX;
  for (size_t index = 0; index < sizeof(INVALID) / sizeof(*INVALID); index++) {
    free(autopilot_argument_list_take(&command.arguments, 1));
    const char *token = *(const char *const *)checked_storage_at_const(
        INVALID, sizeof(INVALID) / sizeof(*INVALID), sizeof(*INVALID), index);
    autopilot_argument_list_set(&command.arguments, 1, strdup(token));
    if (autopilot_order_decode(&command, &output) !=
        BTECH_AUTOPILOT_INVALID_ORDER_QUEUE)
      valid = false;
  }
  autopilot_argument_list_destroy(&command.arguments);
  if (!valid)
    return 1;

  command = (AutopilotCommand){.argcount = 2, .command_enum = GOAL_GOTO};
  autopilot_argument_list_initialize(&command.arguments, AUTOPILOT_MAX_ARGS);
  autopilot_argument_list_set(&command.arguments, 0, strdup("goto"));
  /* Narrowing this long to int before checking its range would accept zero. */
  autopilot_argument_list_set(&command.arguments, 1, strdup("4294967296"));
  autopilot_argument_list_set(&command.arguments, 2, strdup("0"));
  valid = autopilot_order_decode(&command, &output) ==
          BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  autopilot_argument_list_destroy(&command.arguments);
  return valid ? 0 : 2;
}

static int test_malformed_decode(void) {
  BtechAutopilotOrder output;
  AutopilotCommand command = {.argcount = 1, .command_enum = COMMAND_SPEED};
  autopilot_argument_list_initialize(&command.arguments, AUTOPILOT_MAX_ARGS);
  autopilot_argument_list_set(&command.arguments, 0, strdup("speed"));
  autopilot_argument_list_set(&command.arguments, 1, strdup("50x"));
  if (autopilot_order_decode(&command, &output) !=
      BTECH_AUTOPILOT_INVALID_ORDER_QUEUE) {
    autopilot_argument_list_destroy(&command.arguments);
    return 1;
  }
  free(autopilot_argument_list_take(&command.arguments, 1));
  autopilot_argument_list_set(&command.arguments, 1, strdup("50"));
  command.argcount = 0;
  if (autopilot_order_decode(&command, &output) !=
      BTECH_AUTOPILOT_INVALID_ORDER_QUEUE) {
    autopilot_argument_list_destroy(&command.arguments);
    return 2;
  }
  command.argcount = 1;
  command.command_enum = COMMAND_JUMP;
  if (autopilot_order_decode(&command, &output) !=
      BTECH_AUTOPILOT_INVALID_ORDER_QUEUE) {
    autopilot_argument_list_destroy(&command.arguments);
    return 3;
  }
  command.command_enum = COMMAND_SPEED;
  free(autopilot_argument_list_take(&command.arguments, 1));
  autopilot_argument_list_set(&command.arguments, 1,
                              strdup("999999999999999999999999"));
  if (autopilot_order_decode(&command, &output) !=
      BTECH_AUTOPILOT_INVALID_ORDER_QUEUE) {
    autopilot_argument_list_destroy(&command.arguments);
    return 4;
  }
  free(autopilot_argument_list_take(&command.arguments, 1));
  autopilot_argument_list_set(&command.arguments, 1, strdup("050"));
  if (autopilot_order_decode(&command, &output) !=
      BTECH_AUTOPILOT_INVALID_ORDER_QUEUE) {
    autopilot_argument_list_destroy(&command.arguments);
    return 5;
  }
  autopilot_argument_list_destroy(&command.arguments);
  return 0;
}

int main(void) {
  char context_storage;
  BtechContext *context = (BtechContext *)&context_storage;
  const int round_trips = test_round_trips(context);
  if (round_trips != 0)
    return 10 + round_trips;
  const int validation = test_validation(context);
  if (validation != 0)
    return 40 + validation;
  const int enterbase_legacy = test_enterbase_legacy_decode(context);
  if (enterbase_legacy != 0)
    return 60 + enterbase_legacy;
  const int numeric = test_numeric_decode();
  if (numeric != 0)
    return 130 + numeric;
  const int malformed = test_malformed_decode();
  return malformed == 0 ? 0 : 140 + malformed;
}
