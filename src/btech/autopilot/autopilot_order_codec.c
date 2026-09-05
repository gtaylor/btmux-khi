/* Converts public typed orders to and from the legacy execution queue. */

#include "autopilot_order_codec_api.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autopilot.h"
#include "autopilot_argument_list_api.h"
#include "autopilot_commands_api.h"
#include "btech/autopilot.h"
#include "btech/ids.h"
#include "mux/support/checked_storage.h"
#include "registry_api.h"

typedef struct OrderDefinition {
  BtechAutopilotOrderType type;
  int command_enum;
} OrderDefinition;

static const OrderDefinition ORDER_DEFINITIONS[] = {
    {BTECH_AUTOPILOT_ORDER_CHASETARGET, GOAL_CHASETARGET},
    {BTECH_AUTOPILOT_ORDER_DUMBFOLLOW, GOAL_DUMBFOLLOW},
    {BTECH_AUTOPILOT_ORDER_DUMBGOTO, GOAL_DUMBGOTO},
    {BTECH_AUTOPILOT_ORDER_ENTERBASE, GOAL_ENTERBASE},
    {BTECH_AUTOPILOT_ORDER_FOLLOW, GOAL_FOLLOW},
    {BTECH_AUTOPILOT_ORDER_GOTO, GOAL_GOTO},
    {BTECH_AUTOPILOT_ORDER_LEAVEBASE, GOAL_LEAVEBASE},
    {BTECH_AUTOPILOT_ORDER_OLDGOTO, GOAL_OLDGOTO},
    {BTECH_AUTOPILOT_ORDER_ROAM, GOAL_ROAM},
    {BTECH_AUTOPILOT_ORDER_AUTOGUN, COMMAND_AUTOGUN},
    {BTECH_AUTOPILOT_ORDER_DROPOFF, COMMAND_DROPOFF},
    {BTECH_AUTOPILOT_ORDER_EMBARK, COMMAND_EMBARK},
    {BTECH_AUTOPILOT_ORDER_PICKUP, COMMAND_PICKUP},
    {BTECH_AUTOPILOT_ORDER_SHUTDOWN, COMMAND_SHUTDOWN},
    {BTECH_AUTOPILOT_ORDER_SPEED, COMMAND_SPEED},
    {BTECH_AUTOPILOT_ORDER_STARTUP, COMMAND_STARTUP},
    {BTECH_AUTOPILOT_ORDER_UDISEMBARK, COMMAND_UDISEMBARK},
};

static const OrderDefinition *
definition_for_type(BtechAutopilotOrderType type) {
  for (size_t index = 0;
       index < sizeof(ORDER_DEFINITIONS) / sizeof(*ORDER_DEFINITIONS);
       index++) {
    const OrderDefinition *definition = checked_storage_at_const(
        ORDER_DEFINITIONS,
        sizeof(ORDER_DEFINITIONS) / sizeof(*ORDER_DEFINITIONS),
        sizeof(OrderDefinition), index);
    if (definition->type == type)
      return definition;
  }
  return nullptr;
}

static const OrderDefinition *definition_for_enum(int command_enum) {
  for (size_t index = 0;
       index < sizeof(ORDER_DEFINITIONS) / sizeof(*ORDER_DEFINITIONS);
       index++) {
    const OrderDefinition *definition = checked_storage_at_const(
        ORDER_DEFINITIONS,
        sizeof(ORDER_DEFINITIONS) / sizeof(*ORDER_DEFINITIONS),
        sizeof(OrderDefinition), index);
    if (definition->command_enum == command_enum)
      return definition;
  }
  return nullptr;
}

static const AutopilotCommandDefinition *queue_definition(int command_enum) {
  for (int index = 0;; index++) {
    const AutopilotCommandDefinition *definition =
        autopilot_command_definition_at(index);
    if (definition->name == nullptr)
      return nullptr;
    if (definition->command_enum == command_enum)
      return definition;
  }
}

static bool target_is_valid(BtechContext *context, BtechObjectId target) {
  return (context != nullptr && btech_context_is_mech(context, target) &&
          btech_context_get_mech(context, target) != nullptr) != 0;
}

static bool parse_enter_base_direction(const char *value,
                                       BtechAutopilotDirection *direction) {
  if (value == nullptr || value[0] == '\0' || direction == nullptr)
    return false;
  /* Legacy execution selects the direction from this first ASCII byte. */
  switch (value[0]) {
  case 'n':
  case 'N':
    *direction = BTECH_AUTOPILOT_NORTH;
    break;
  case 'e':
  case 'E':
    *direction = BTECH_AUTOPILOT_EAST;
    break;
  case 's':
  case 'S':
    *direction = BTECH_AUTOPILOT_SOUTH;
    break;
  case 'w':
  case 'W':
    *direction = BTECH_AUTOPILOT_WEST;
    break;
  default:
    return false;
  }
  return true;
}

BtechAutopilotResult
btech_autopilot_order_validate(BtechContext *context,
                               const BtechAutopilotOrder *order) {
  if (context == nullptr || order == nullptr ||
      definition_for_type(order->type) == nullptr)
    return BTECH_AUTOPILOT_INVALID_ARGUMENT;
  switch (order->type) {
  case BTECH_AUTOPILOT_ORDER_CHASETARGET:
  case BTECH_AUTOPILOT_ORDER_DUMBFOLLOW:
  case BTECH_AUTOPILOT_ORDER_FOLLOW:
  case BTECH_AUTOPILOT_ORDER_EMBARK:
  case BTECH_AUTOPILOT_ORDER_PICKUP:
    if (target_is_valid(context, order->data.target.target))
      return BTECH_AUTOPILOT_OK;
    return BTECH_AUTOPILOT_UNAVAILABLE;
  case BTECH_AUTOPILOT_ORDER_DUMBGOTO:
  case BTECH_AUTOPILOT_ORDER_GOTO:
  case BTECH_AUTOPILOT_ORDER_OLDGOTO:
    return order->data.destination.x >= 0 && order->data.destination.y >= 0
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ARGUMENT;
  case BTECH_AUTOPILOT_ORDER_ENTERBASE:
    return order->data.enter_base.direction >= BTECH_AUTOPILOT_NORTH &&
                   order->data.enter_base.direction <= BTECH_AUTOPILOT_WEST
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ARGUMENT;
  case BTECH_AUTOPILOT_ORDER_LEAVEBASE:
    return order->data.leave_base.heading >= 0 &&
                   order->data.leave_base.heading <= 359
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ARGUMENT;
  case BTECH_AUTOPILOT_ORDER_ROAM:
    if (order->data.roam.mode == BTECH_AUTOPILOT_ROAM_MAP)
      return BTECH_AUTOPILOT_OK;
    return order->data.roam.mode == BTECH_AUTOPILOT_ROAM_RADIUS &&
                   order->data.roam.x >= 0 && order->data.roam.y >= 0 &&
                   order->data.roam.radius >= 1 &&
                   order->data.roam.radius <= AUTO_ROAM_MAX_RADIUS
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ARGUMENT;
  case BTECH_AUTOPILOT_ORDER_AUTOGUN:
    if (order->data.autogun.mode == BTECH_AUTOPILOT_AUTOGUN_AUTOMATIC ||
        order->data.autogun.mode == BTECH_AUTOPILOT_AUTOGUN_OFF)
      return BTECH_AUTOPILOT_OK;
    if (order->data.autogun.mode != BTECH_AUTOPILOT_AUTOGUN_TARGET)
      return BTECH_AUTOPILOT_INVALID_ARGUMENT;
    if (target_is_valid(context, order->data.autogun.target))
      return BTECH_AUTOPILOT_OK;
    return BTECH_AUTOPILOT_UNAVAILABLE;
  case BTECH_AUTOPILOT_ORDER_SPEED:
    return order->data.speed.percent >= 1 && order->data.speed.percent <= 100
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ARGUMENT;
  case BTECH_AUTOPILOT_ORDER_DROPOFF:
  case BTECH_AUTOPILOT_ORDER_SHUTDOWN:
  case BTECH_AUTOPILOT_ORDER_STARTUP:
  case BTECH_AUTOPILOT_ORDER_UDISEMBARK:
    return BTECH_AUTOPILOT_OK;
  }
  return BTECH_AUTOPILOT_INVALID_ARGUMENT;
}

static BtechAutopilotResult argument_set(AutopilotArgumentList *arguments,
                                         size_t index, const char *value) {
  char *copy = strdup(value);
  if (copy == nullptr)
    return BTECH_AUTOPILOT_NO_MEMORY;
  autopilot_argument_list_set(arguments, index, copy);
  return BTECH_AUTOPILOT_OK;
}

BtechAutopilotResult
autopilot_order_encode(BtechContext *context, const BtechAutopilotOrder *order,
                       const AutopilotCommandDefinition **definition,
                       AutopilotArgumentList *arguments) {
  if (definition == nullptr || arguments == nullptr)
    return BTECH_AUTOPILOT_INVALID_ARGUMENT;
  const BtechAutopilotResult VALID =
      btech_autopilot_order_validate(context, order);
  if (VALID != BTECH_AUTOPILOT_OK)
    return VALID;
  const OrderDefinition *mapping = definition_for_type(order->type);
  *definition = queue_definition(mapping->command_enum);
  if (*definition == nullptr)
    return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;

  autopilot_argument_list_initialize(arguments, AUTOPILOT_MAX_ARGS);
  BtechAutopilotResult result = argument_set(arguments, 0, (*definition)->name);
  char first[96] = {0};
  char second[32] = {0};
  if (result != BTECH_AUTOPILOT_OK)
    return result;
  switch (order->type) {
  case BTECH_AUTOPILOT_ORDER_CHASETARGET:
  case BTECH_AUTOPILOT_ORDER_DUMBFOLLOW:
  case BTECH_AUTOPILOT_ORDER_FOLLOW:
  case BTECH_AUTOPILOT_ORDER_EMBARK:
  case BTECH_AUTOPILOT_ORDER_PICKUP:
    (void)snprintf(first, sizeof(first), "%ld", order->data.target.target);
    return argument_set(arguments, 1, first);
  case BTECH_AUTOPILOT_ORDER_DUMBGOTO:
  case BTECH_AUTOPILOT_ORDER_GOTO:
  case BTECH_AUTOPILOT_ORDER_OLDGOTO:
    (void)snprintf(first, sizeof(first), "%d", order->data.destination.x);
    (void)snprintf(second, sizeof(second), "%d", order->data.destination.y);
    result = argument_set(arguments, 1, first);
    return result == BTECH_AUTOPILOT_OK ? argument_set(arguments, 2, second)
                                        : result;
  case BTECH_AUTOPILOT_ORDER_ENTERBASE: {
    const char *direction = nullptr;
    switch (order->data.enter_base.direction) {
    case BTECH_AUTOPILOT_NORTH:
      direction = "north";
      break;
    case BTECH_AUTOPILOT_EAST:
      direction = "east";
      break;
    case BTECH_AUTOPILOT_SOUTH:
      direction = "south";
      break;
    case BTECH_AUTOPILOT_WEST:
      direction = "west";
      break;
    }
    return argument_set(arguments, 1, direction);
  }
  case BTECH_AUTOPILOT_ORDER_LEAVEBASE:
    (void)snprintf(first, sizeof(first), "%d", order->data.leave_base.heading);
    return argument_set(arguments, 1, first);
  case BTECH_AUTOPILOT_ORDER_ROAM:
    if (order->data.roam.mode == BTECH_AUTOPILOT_ROAM_MAP)
      return argument_set(arguments, 1, "map");
    (void)snprintf(first, sizeof(first), "radius %d %d %d", order->data.roam.x,
                   order->data.roam.y, order->data.roam.radius);
    return argument_set(arguments, 1, first);
  case BTECH_AUTOPILOT_ORDER_AUTOGUN:
    if (order->data.autogun.mode == BTECH_AUTOPILOT_AUTOGUN_AUTOMATIC)
      return argument_set(arguments, 1, "on");
    if (order->data.autogun.mode == BTECH_AUTOPILOT_AUTOGUN_OFF)
      return argument_set(arguments, 1, "off");
    (void)snprintf(first, sizeof(first), "target %ld",
                   order->data.autogun.target);
    return argument_set(arguments, 1, first);
  case BTECH_AUTOPILOT_ORDER_SPEED:
    (void)snprintf(first, sizeof(first), "%d", order->data.speed.percent);
    return argument_set(arguments, 1, first);
  case BTECH_AUTOPILOT_ORDER_DROPOFF:
  case BTECH_AUTOPILOT_ORDER_SHUTDOWN:
  case BTECH_AUTOPILOT_ORDER_STARTUP:
  case BTECH_AUTOPILOT_ORDER_UDISEMBARK:
    return BTECH_AUTOPILOT_OK;
  }
  return BTECH_AUTOPILOT_INVALID_ARGUMENT;
}

static bool parse_long_exact(const char *text, long *value) {
  if (text == nullptr || *text == '\0')
    return false;
  char *end = nullptr;
  errno = 0;
  const long PARSED = strtol(text, &end, 10);
  if (errno == ERANGE || end == text || *end != '\0')
    return false;
  char canonical[32];
  (void)snprintf(canonical, sizeof(canonical), "%ld", PARSED);
  if (strcmp(text, canonical) != 0)
    return false;
  *value = PARSED;
  return true;
}

static bool parse_int_exact(const char *text, int *value) {
  long parsed;
  if (!parse_long_exact(text, &parsed) || parsed < INT_MIN || parsed > INT_MAX)
    return false;
  *value = (int)parsed;
  return true;
}

static bool parse_radius(const char *text, int *x, int *y, int *radius) {
  if (text == nullptr)
    return false;
  char *copy = strdup(text);
  if (copy == nullptr)
    return false;
  char *save = nullptr;
  const char *name = strtok_r(copy, " ", &save);
  const char *x_text = strtok_r(nullptr, " ", &save);
  const char *y_text = strtok_r(nullptr, " ", &save);
  const char *radius_text = strtok_r(nullptr, " ", &save);
  const bool PARSED =
      (name != nullptr && strcmp(name, "radius") == 0 && x_text != nullptr &&
       y_text != nullptr && radius_text != nullptr &&
       strtok_r(nullptr, " ", &save) == nullptr && parse_int_exact(x_text, x) &&
       parse_int_exact(y_text, y) && parse_int_exact(radius_text, radius)) != 0;
  char canonical[96] = {0};
  if (PARSED)
    (void)snprintf(canonical, sizeof(canonical), "radius %d %d %d", *x, *y,
                   *radius);
  const bool RESULT = (PARSED && strcmp(text, canonical) == 0) != 0;
  free(copy);
  return RESULT;
}

static bool parse_target_mode(const char *text, BtechObjectId *target) {
  if (text == nullptr)
    return false;
  char *copy = strdup(text);
  if (copy == nullptr)
    return false;
  char *save = nullptr;
  const char *name = strtok_r(copy, " ", &save);
  const char *target_text = strtok_r(nullptr, " ", &save);
  const bool PARSED =
      (name != nullptr && strcmp(name, "target") == 0 &&
       target_text != nullptr && strtok_r(nullptr, " ", &save) == nullptr &&
       parse_long_exact(target_text, target)) != 0;
  char canonical[64] = {0};
  if (PARSED)
    (void)snprintf(canonical, sizeof(canonical), "target %ld", *target);
  const bool RESULT = (PARSED && strcmp(text, canonical) == 0) != 0;
  free(copy);
  return RESULT;
}

BtechAutopilotResult autopilot_order_decode(const AutopilotCommand *command,
                                            BtechAutopilotOrder *order) {
  if (command == nullptr || order == nullptr)
    return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  const OrderDefinition *mapping = definition_for_enum(command->command_enum);
  const AutopilotCommandDefinition *definition =
      mapping == nullptr ? nullptr : queue_definition(mapping->command_enum);
  if (definition == nullptr ||
      command->arguments.capacity != AUTOPILOT_MAX_ARGS ||
      command->argcount != (unsigned char)definition->argcount)
    return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  for (size_t index = 0; index < AUTOPILOT_MAX_ARGS; index++) {
    const char *argument =
        autopilot_argument_list_get(&command->arguments, index);
    if ((argument != nullptr) != (index <= (size_t)definition->argcount))
      return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  }
  if (strcmp(autopilot_argument_list_get(&command->arguments, 0),
             definition->name) != 0)
    return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  memset(order, 0, sizeof(*order));
  order->type = mapping->type;
  const char *first = autopilot_argument_list_get(&command->arguments, 1);
  const char *second = autopilot_argument_list_get(&command->arguments, 2);
  switch (order->type) {
  case BTECH_AUTOPILOT_ORDER_CHASETARGET:
  case BTECH_AUTOPILOT_ORDER_DUMBFOLLOW:
  case BTECH_AUTOPILOT_ORDER_FOLLOW:
  case BTECH_AUTOPILOT_ORDER_EMBARK:
  case BTECH_AUTOPILOT_ORDER_PICKUP:
    if (parse_long_exact(first, &order->data.target.target))
      return BTECH_AUTOPILOT_OK;
    return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  case BTECH_AUTOPILOT_ORDER_DUMBGOTO:
  case BTECH_AUTOPILOT_ORDER_GOTO:
  case BTECH_AUTOPILOT_ORDER_OLDGOTO:
    return parse_int_exact(first, &order->data.destination.x) &&
                   parse_int_exact(second, &order->data.destination.y) &&
                   order->data.destination.x >= 0 &&
                   order->data.destination.y >= 0
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  case BTECH_AUTOPILOT_ORDER_ENTERBASE:
    return parse_enter_base_direction(first, &order->data.enter_base.direction)
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  case BTECH_AUTOPILOT_ORDER_LEAVEBASE:
    return parse_int_exact(first, &order->data.leave_base.heading) &&
                   order->data.leave_base.heading >= 0 &&
                   order->data.leave_base.heading <= 359
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  case BTECH_AUTOPILOT_ORDER_ROAM: {
    if (first != nullptr && strcmp(first, "map") == 0) {
      order->data.roam.mode = BTECH_AUTOPILOT_ROAM_MAP;
      return BTECH_AUTOPILOT_OK;
    }
    order->data.roam.mode = BTECH_AUTOPILOT_ROAM_RADIUS;
    return parse_radius(first, &order->data.roam.x, &order->data.roam.y,
                        &order->data.roam.radius) &&
                   order->data.roam.x >= 0 && order->data.roam.y >= 0 &&
                   order->data.roam.radius >= 1 &&
                   order->data.roam.radius <= AUTO_ROAM_MAX_RADIUS
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  }
  case BTECH_AUTOPILOT_ORDER_AUTOGUN: {
    if (first != nullptr && strcmp(first, "on") == 0)
      order->data.autogun.mode = BTECH_AUTOPILOT_AUTOGUN_AUTOMATIC;
    else if (first != nullptr && strcmp(first, "off") == 0)
      order->data.autogun.mode = BTECH_AUTOPILOT_AUTOGUN_OFF;
    else if (parse_target_mode(first, &order->data.autogun.target))
      order->data.autogun.mode = BTECH_AUTOPILOT_AUTOGUN_TARGET;
    else
      return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
    return BTECH_AUTOPILOT_OK;
  }
  case BTECH_AUTOPILOT_ORDER_SPEED:
    return parse_int_exact(first, &order->data.speed.percent) &&
                   order->data.speed.percent >= 1 &&
                   order->data.speed.percent <= 100
               ? BTECH_AUTOPILOT_OK
               : BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  case BTECH_AUTOPILOT_ORDER_DROPOFF:
  case BTECH_AUTOPILOT_ORDER_SHUTDOWN:
  case BTECH_AUTOPILOT_ORDER_STARTUP:
  case BTECH_AUTOPILOT_ORDER_UDISEMBARK:
    return BTECH_AUTOPILOT_OK;
  }
  return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
}
