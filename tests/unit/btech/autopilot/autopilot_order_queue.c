#include <string.h>

#include "autopilot.h"
#include "autopilot_argument_list_api.h"
#include "autopilot_order_queue_api.h"
#include "mux/support/doubly_linked_list.h"

static void arguments_set(AutopilotArgumentList *arguments, const char *name,
                          const char *value) {
  autopilot_argument_list_initialize(arguments, AUTOPILOT_MAX_ARGS);
  autopilot_argument_list_set(arguments, 0, strdup(name));
  if (value != nullptr)
    autopilot_argument_list_set(arguments, 1, strdup(value));
}

static void wait_arguments_set(AutopilotArgumentList *arguments) {
  arguments_set(arguments, "wait", "10");
  autopilot_argument_list_set(arguments, 2, strdup("seconds"));
}

int main(void) {
  Autopilot autopilot = {.commands = doubly_linked_list_create_list()};
  AutopilotArgumentList arguments;
  const AutopilotCommandDefinition SPEED = {
      .name = "speed", .argcount = 1, .command_enum = COMMAND_SPEED};
  const AutopilotCommandDefinition WAIT = {
      .name = "wait", .argcount = 2, .command_enum = GOAL_WAIT};
  const AutopilotCommandDefinition JUMP = {
      .name = "jump", .argcount = 1, .command_enum = COMMAND_JUMP};

  wait_arguments_set(&arguments);
  if (autopilot_order_enqueue(&autopilot, &WAIT, &arguments) !=
      AUTOPILOT_ORDER_UNSUPPORTED)
    return 1;
  autopilot_argument_list_destroy(&arguments);

  arguments_set(&arguments, "jump", "90");
  if (autopilot_order_enqueue(&autopilot, &JUMP, &arguments) !=
          AUTOPILOT_ORDER_UNSUPPORTED ||
      autopilot_order_count(&autopilot) != 0)
    return 2;
  autopilot_argument_list_destroy(&arguments);

  arguments_set(&arguments, "speed", nullptr);
  if (autopilot_order_enqueue(&autopilot, &SPEED, &arguments) !=
      AUTOPILOT_ORDER_INVALID)
    return 3;
  autopilot_argument_list_destroy(&arguments);

  for (int index = 0; index < AUTOPILOT_MEMORY; index++) {
    arguments_set(&arguments, "speed", "50");
    if (autopilot_order_enqueue(&autopilot, &SPEED, &arguments) !=
        AUTOPILOT_ORDER_OK)
      return 4;
    autopilot_argument_list_destroy(&arguments);
  }
  if (autopilot_order_count(&autopilot) != AUTOPILOT_MEMORY)
    return 5;
  arguments_set(&arguments, "speed", "60");
  if (autopilot_order_enqueue(&autopilot, &SPEED, &arguments) !=
      AUTOPILOT_ORDER_FULL)
    return 6;
  autopilot_argument_list_destroy(&arguments);

  const AutopilotCommand *command = autopilot_order_at(&autopilot, 0);
  if (command == nullptr || command->command_enum != COMMAND_SPEED ||
      strcmp(autopilot_argument_list_get(&command->arguments, 1), "50") != 0)
    return 7;
  if (autopilot_order_remove(&autopilot, AUTOPILOT_MEMORY) !=
          AUTOPILOT_ORDER_NOT_FOUND ||
      autopilot_order_remove(&autopilot, 1) != AUTOPILOT_ORDER_OK ||
      autopilot_order_count(&autopilot) != AUTOPILOT_MEMORY - 1)
    return 8;
  autopilot_order_clear(&autopilot);
  if (autopilot_order_count(&autopilot) != 0 ||
      autopilot_order_pop(&autopilot) != AUTOPILOT_ORDER_NOT_FOUND)
    return 9;
  doubly_linked_list_destroy_list(autopilot.commands);
  return 0;
}
