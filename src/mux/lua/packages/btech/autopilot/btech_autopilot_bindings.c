/* Typed Lua bindings for BattleTech autopilot administration. */

#include <limits.h>
#include <lua.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

#include "btech/autopilot.h"
#include "btech/ids.h"
#include "btech/special_objects.h"
#include "mux/lua/lua_error.h"
#include "mux/lua/lua_error_codes.h"
#include "mux/lua/packages/btech/btech_constants.h"
#include "mux/lua/packages/btech/btech_package.h"
#include "mux/lua/packages/btech/btech_package_internal.h"
#include "mux/server/platform.h"
#include "mux/support/checked_storage.h"

/**
 * @par LuaLS definition btech type btech.autopilot.orders
 * @code{.lua}
 * ---Typed order-name constant from [`btech.autopilot.orders`](lua://btech.autopilot.orders).
 * ---@class BtechAutopilotOrderName
 * ---Typed base-entry direction from [`btech.autopilot.directions`](lua://btech.autopilot.directions).
 * ---@class BtechAutopilotDirection
 * ---Typed roaming mode from [`btech.autopilot.roam_modes`](lua://btech.autopilot.roam_modes).
 * ---@class BtechAutopilotRoamMode
 * ---Typed automatic-gunnery mode from [`btech.autopilot.autogun_modes`](lua://btech.autopilot.autogun_modes).
 * ---@class BtechAutopilotAutogunMode
 * ---@class (exact) BtechAutopilotOrderNamespace
 * ---@field CHASE_TARGET BtechAutopilotOrderName
 * ---@field DUMB_FOLLOW BtechAutopilotOrderName
 * ---@field FOLLOW BtechAutopilotOrderName
 * ---@field EMBARK BtechAutopilotOrderName
 * ---@field PICK_UP BtechAutopilotOrderName
 * ---@field DUMB_GOTO BtechAutopilotOrderName
 * ---@field GOTO BtechAutopilotOrderName
 * ---@field OLD_GOTO BtechAutopilotOrderName
 * ---@field ENTER_BASE BtechAutopilotOrderName
 * ---@field LEAVE_BASE BtechAutopilotOrderName
 * ---@field ROAM BtechAutopilotOrderName
 * ---@field AUTO_GUN BtechAutopilotOrderName
 * ---@field DROP_OFF BtechAutopilotOrderName
 * ---@field SHUT_DOWN BtechAutopilotOrderName
 * ---@field START_UP BtechAutopilotOrderName
 * ---@field UNIT_DISEMBARK BtechAutopilotOrderName
 * ---@field SPEED BtechAutopilotOrderName
 * ---@class (exact) BtechAutopilotDirectionNamespace
 * ---@field NORTH BtechAutopilotDirection
 * ---@field EAST BtechAutopilotDirection
 * ---@field SOUTH BtechAutopilotDirection
 * ---@field WEST BtechAutopilotDirection
 * ---@class (exact) BtechAutopilotRoamModeNamespace
 * ---@field MAP BtechAutopilotRoamMode
 * ---@field RADIUS BtechAutopilotRoamMode
 * ---@class (exact) BtechAutopilotAutogunModeNamespace
 * ---@field AUTOMATIC BtechAutopilotAutogunMode
 * ---@field OFF BtechAutopilotAutogunMode
 * ---@field TARGET BtechAutopilotAutogunMode
 * ---@class BtechAutopilotTargetOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field target DbRef|Object
 * ---@class BtechAutopilotDestinationOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field x integer Nonnegative coordinate (0 through 2147483647).
 * ---@field y integer Nonnegative coordinate (0 through 2147483647).
 * ---@class BtechAutopilotEnterBaseOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field direction BtechAutopilotDirection
 * ---@class BtechAutopilotLeaveBaseOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field heading integer Heading from 0 through 359.
 * ---@class BtechAutopilotMapRoamOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field mode BtechAutopilotRoamMode
 * ---@class BtechAutopilotRadiusRoamOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field mode BtechAutopilotRoamMode
 * ---@field x integer Nonnegative coordinate (0 through 2147483647).
 * ---@field y integer Nonnegative coordinate (0 through 2147483647).
 * ---@field radius integer Radius from 1 through 30.
 * ---@class BtechAutopilotAutomaticAutogunOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field mode BtechAutopilotAutogunMode
 * ---@class BtechAutopilotTargetAutogunOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field mode BtechAutopilotAutogunMode
 * ---@field target DbRef|Object
 * ---@class BtechAutopilotSimpleOrder
 * ---@field name BtechAutopilotOrderName
 * ---@class BtechAutopilotSpeedOrder
 * ---@field name BtechAutopilotOrderName
 * ---@field percent integer Speed percentage from 1 through 100.
 * ---@class BtechAutopilotResolvedTargetOrder: BtechAutopilotTargetOrder
 * ---@field target Object
 * ---@class BtechAutopilotResolvedTargetAutogunOrder: BtechAutopilotTargetAutogunOrder
 * ---@field target Object
 * @endcode
 * @par LuaLS definition btech alias btech.autopilot.order
 * @code{.lua}
 * ---@alias BtechAutopilotOrder BtechAutopilotTargetOrder|BtechAutopilotDestinationOrder|BtechAutopilotEnterBaseOrder|BtechAutopilotLeaveBaseOrder|BtechAutopilotMapRoamOrder|BtechAutopilotRadiusRoamOrder|BtechAutopilotAutomaticAutogunOrder|BtechAutopilotTargetAutogunOrder|BtechAutopilotSimpleOrder|BtechAutopilotSpeedOrder
 * ---@alias BtechAutopilotStatusOrder BtechAutopilotResolvedTargetOrder|BtechAutopilotDestinationOrder|BtechAutopilotEnterBaseOrder|BtechAutopilotLeaveBaseOrder|BtechAutopilotMapRoamOrder|BtechAutopilotRadiusRoamOrder|BtechAutopilotAutomaticAutogunOrder|BtechAutopilotResolvedTargetAutogunOrder|BtechAutopilotSimpleOrder|BtechAutopilotSpeedOrder
 * @endcode
 * @par LuaLS definition btech type btech.autopilot.records
 * @code{.lua}
 * ---@class BtechAutopilotModes
 * ---@field autogun_enabled boolean
 * ---@field autogun_suspended boolean
 * ---@field piloting_suspended boolean
 * ---@field roaming boolean
 * ---@field manual_sensors boolean
 * ---@field chasing_target boolean
 * ---@field chase_resume_pending boolean
 * ---@field swarm_charging boolean
 * ---@field assigned_target boolean
 * ---@class BtechAutopilotStatus
 * ---@field container_unit? Object
 * ---@field associated_unit? Object
 * ---@field map? Object
 * ---@field association "none"|"ready"|"associated"|"conflict"
 * ---@field engaged boolean
 * ---@field speed_percent integer
 * ---@field order_count integer
 * ---@field order_capacity integer Always 100.
 * ---@field orders BtechAutopilotStatusOrder[]
 * ---@field modes BtechAutopilotModes
 * ---@class BtechAutopilotEventStats
 * ---@field goto integer
 * ---@field leave integer
 * ---@field command integer
 * ---@field autogun integer
 * ---@field sensor integer
 * ---@field follow integer
 * ---@field enter_base integer
 * ---@field reply integer
 * ---@field profile integer
 * ---@field roam integer
 * ---@field total integer
 * @endcode
 * @par LuaLS definition btech callable btech.autopilot.status
 * @code{.lua}
 * ---@param autopilot DbRef|Object
 * ---@return BtechAutopilotStatus status
 * function btech_autopilot.status(autopilot, ...) end
 * @endcode
 * @par LuaLS definition btech callable btech.autopilot.add_order
 * @code{.lua}
 * ---@param autopilot DbRef|Object
 * ---@param order BtechAutopilotOrder
 * function btech_autopilot.add_order(autopilot, order, ...) end
 * @endcode
 * @par LuaLS definition btech callable btech.autopilot.remove_order
 * @code{.lua}
 * ---@param autopilot DbRef|Object
 * ---@param position integer
 * function btech_autopilot.remove_order(autopilot, position, ...) end
 * @endcode
 * @par LuaLS definition btech callable btech.autopilot.clear_orders
 * @code{.lua}
 * ---@param autopilot DbRef|Object
 * function btech_autopilot.clear_orders(autopilot, ...) end
 * @endcode
 * @par LuaLS definition btech callable btech.autopilot.engage
 * @code{.lua}
 * ---@param autopilot DbRef|Object
 * function btech_autopilot.engage(autopilot, ...) end
 * @endcode
 * @par LuaLS definition btech callable btech.autopilot.disengage
 * @code{.lua}
 * ---@param autopilot DbRef|Object
 * function btech_autopilot.disengage(autopilot, ...) end
 * @endcode
 * @par LuaLS definition btech callable btech.autopilot.event_stats
 * @code{.lua}
 * ---@param autopilot DbRef|Object
 * ---@return BtechAutopilotEventStats stats
 * function btech_autopilot.event_stats(autopilot, ...) end
 * @endcode
 */

static const char *const TARGET_FIELDS[] = {"name", "target"};
static const char *const DESTINATION_FIELDS[] = {"name", "x", "y"};
static const char *const DIRECTION_FIELDS[] = {"name", "direction"};
static const char *const HEADING_FIELDS[] = {"name", "heading"};
static const char *const ROAM_MAP_FIELDS[] = {"name", "mode"};
static const char *const ROAM_RADIUS_FIELDS[] = {"name", "mode", "x", "y",
                                                 "radius"};
static const char *const AUTOGUN_FIELDS[] = {"name", "mode"};
static const char *const AUTOGUN_TARGET_FIELDS[] = {"name", "mode", "target"};
static const char *const NAME_FIELDS[] = {"name"};
static const char *const SPEED_FIELDS[] = {"name", "percent"};

static const BtechLuaConstantEntry ORDER_ENTRIES[] = {
    {"CHASE_TARGET", BTECH_AUTOPILOT_ORDER_CHASETARGET},
    {"DUMB_FOLLOW", BTECH_AUTOPILOT_ORDER_DUMBFOLLOW},
    {"FOLLOW", BTECH_AUTOPILOT_ORDER_FOLLOW},
    {"EMBARK", BTECH_AUTOPILOT_ORDER_EMBARK},
    {"PICK_UP", BTECH_AUTOPILOT_ORDER_PICKUP},
    {"DUMB_GOTO", BTECH_AUTOPILOT_ORDER_DUMBGOTO},
    {"GOTO", BTECH_AUTOPILOT_ORDER_GOTO},
    {"OLD_GOTO", BTECH_AUTOPILOT_ORDER_OLDGOTO},
    {"ENTER_BASE", BTECH_AUTOPILOT_ORDER_ENTERBASE},
    {"LEAVE_BASE", BTECH_AUTOPILOT_ORDER_LEAVEBASE},
    {"ROAM", BTECH_AUTOPILOT_ORDER_ROAM},
    {"AUTO_GUN", BTECH_AUTOPILOT_ORDER_AUTOGUN},
    {"DROP_OFF", BTECH_AUTOPILOT_ORDER_DROPOFF},
    {"SHUT_DOWN", BTECH_AUTOPILOT_ORDER_SHUTDOWN},
    {"START_UP", BTECH_AUTOPILOT_ORDER_STARTUP},
    {"UNIT_DISEMBARK", BTECH_AUTOPILOT_ORDER_UDISEMBARK},
    {"SPEED", BTECH_AUTOPILOT_ORDER_SPEED},
};
static const BtechLuaConstantCatalog ORDER_CATALOG = {
    .qualified_name = "btech.autopilot.orders",
    .entries = ORDER_ENTRIES,
    .entry_count = sizeof(ORDER_ENTRIES) / sizeof(*ORDER_ENTRIES),
};

static const BtechLuaConstantEntry DIRECTION_ENTRIES[] = {
    {"NORTH", BTECH_AUTOPILOT_NORTH},
    {"EAST", BTECH_AUTOPILOT_EAST},
    {"SOUTH", BTECH_AUTOPILOT_SOUTH},
    {"WEST", BTECH_AUTOPILOT_WEST},
};
static const BtechLuaConstantCatalog DIRECTION_CATALOG = {
    .qualified_name = "btech.autopilot.directions",
    .entries = DIRECTION_ENTRIES,
    .entry_count = sizeof(DIRECTION_ENTRIES) / sizeof(*DIRECTION_ENTRIES),
};

static const BtechLuaConstantEntry ROAM_MODE_ENTRIES[] = {
    {"MAP", BTECH_AUTOPILOT_ROAM_MAP},
    {"RADIUS", BTECH_AUTOPILOT_ROAM_RADIUS},
};
static const BtechLuaConstantCatalog ROAM_MODE_CATALOG = {
    .qualified_name = "btech.autopilot.roam_modes",
    .entries = ROAM_MODE_ENTRIES,
    .entry_count = sizeof(ROAM_MODE_ENTRIES) / sizeof(*ROAM_MODE_ENTRIES),
};

static const BtechLuaConstantEntry AUTOGUN_MODE_ENTRIES[] = {
    {"AUTOMATIC", BTECH_AUTOPILOT_AUTOGUN_AUTOMATIC},
    {"OFF", BTECH_AUTOPILOT_AUTOGUN_OFF},
    {"TARGET", BTECH_AUTOPILOT_AUTOGUN_TARGET},
};
static const BtechLuaConstantCatalog AUTOGUN_MODE_CATALOG = {
    .qualified_name = "btech.autopilot.autogun_modes",
    .entries = AUTOGUN_MODE_ENTRIES,
    .entry_count = sizeof(AUTOGUN_MODE_ENTRIES) / sizeof(*AUTOGUN_MODE_ENTRIES),
};

typedef struct LuaDbRefInput {
  int index;
  int argument;
  const char *label;
} LuaDbRefInput;

static void check_dbref_input(lua_State *state, LuaDbRefInput input) {
  if (lua_type(state, input.index) != LUA_TNUMBER) {
    if (lua_isnumber(state, input.index))
      (void)lua_error_arg(state, input.argument, LUA_ERROR_CODE_OBJECT_INVALID,
                          "%s must be an integer dbref or Object", input.label);
    return;
  }
  const lua_Number NUMBER = lua_tonumber(state, input.index);
  if (!isfinite(NUMBER) || floor(NUMBER) != NUMBER ||
      NUMBER < (lua_Number)LONG_MIN || NUMBER >= (lua_Number)LONG_MAX)
    (void)lua_error_arg(state, input.argument, LUA_ERROR_CODE_OBJECT_INVALID,
                        "%s must be an integer dbref or Object", input.label);
}

static DbRef require_autopilot(lua_State *state, LuaBtechPackage *package) {
  lua_btech_check_arity(state, 1);
  check_dbref_input(state, (LuaDbRefInput){
                               .index = 1,
                               .argument = 1,
                               .label = "autopilot",
                           });
  return lua_btech_require_special(package, state, 1, BTECH_SPECIAL_AUTOPILOT,
                                   "autopilot");
}

static DbRef require_unit_field(lua_State *state, LuaBtechPackage *package,
                                int table, const char *field) {
  lua_btech_get_field(state, table, field);
  check_dbref_input(state, (LuaDbRefInput){
                               .index = -1,
                               .argument = 2,
                               .label = field,
                           });
  lua_pop(state, 1);
  const DbRef OBJECT =
      lua_btech_require_object_field(package, state, table, field, 2);
  if (btech_special_object_type(lua_btech_context(package), OBJECT) !=
      BTECH_SPECIAL_MECH)
    (void)lua_error_arg(state, 2, LUA_ERROR_CODE_OBJECT_INVALID,
                        "%s is not a registered BTech unit", field);
  return OBJECT;
}

static int require_constant_field(lua_State *state, LuaBtechPackage *package,
                                  int table, const char *field,
                                  const BtechLuaConstantCatalog *catalog) {
  lua_btech_get_field(state, table, field);
  const int VALUE =
      lua_btech_constant_require_at(package, state, -1, 2, field, catalog);
  lua_pop(state, 1);
  return VALUE;
}

static void check_order_options(lua_State *state, int table,
                                const char *const *allowed,
                                size_t allowed_count) {
  lua_pushnil(state);
  while (lua_next(state, table) != 0) {
    if (lua_type(state, -2) == LUA_TSTRING) {
      size_t length;
      const char *field = lua_tolstring(state, -2, &length);
      if (memchr(field, '\0', length) != nullptr)
        (void)lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                            "order field names must not contain embedded NUL "
                            "bytes");
    }
    lua_pop(state, 1);
  }
  lua_btech_check_options(state, table, allowed, allowed_count, 2);
}

static void parse_order(lua_State *state, LuaBtechPackage *package,
                        BtechAutopilotOrder *order) {
  if (!lua_istable(state, 2))
    (void)lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                        "order must be a table");
  memset(order, 0, sizeof(*order));
  const int ORDER_TYPE =
      require_constant_field(state, package, 2, "name", &ORDER_CATALOG);
  order->type = (BtechAutopilotOrderType)ORDER_TYPE;
  switch (order->type) {
  case BTECH_AUTOPILOT_ORDER_CHASETARGET:
  case BTECH_AUTOPILOT_ORDER_DUMBFOLLOW:
  case BTECH_AUTOPILOT_ORDER_FOLLOW:
  case BTECH_AUTOPILOT_ORDER_EMBARK:
  case BTECH_AUTOPILOT_ORDER_PICKUP:
    check_order_options(state, 2, TARGET_FIELDS, 2);
    order->data.target.target = require_unit_field(state, package, 2, "target");
    break;
  case BTECH_AUTOPILOT_ORDER_DUMBGOTO:
  case BTECH_AUTOPILOT_ORDER_GOTO:
  case BTECH_AUTOPILOT_ORDER_OLDGOTO:
    check_order_options(state, 2, DESTINATION_FIELDS, 3);
    order->data.destination.x =
        (int)lua_btech_check_integer_field(state, 2, "x", 0, INT_MAX, 2);
    order->data.destination.y =
        (int)lua_btech_check_integer_field(state, 2, "y", 0, INT_MAX, 2);
    break;
  case BTECH_AUTOPILOT_ORDER_ENTERBASE:
    check_order_options(state, 2, DIRECTION_FIELDS, 2);
    const int DIRECTION = require_constant_field(state, package, 2, "direction",
                                                 &DIRECTION_CATALOG);
    order->data.enter_base.direction = (BtechAutopilotDirection)DIRECTION;
    break;
  case BTECH_AUTOPILOT_ORDER_LEAVEBASE:
    check_order_options(state, 2, HEADING_FIELDS, 2);
    order->data.leave_base.heading =
        (int)lua_btech_check_integer_field(state, 2, "heading", 0, 359, 2);
    break;
  case BTECH_AUTOPILOT_ORDER_ROAM:
    const int ROAM_MODE =
        require_constant_field(state, package, 2, "mode", &ROAM_MODE_CATALOG);
    order->data.roam.mode = (BtechAutopilotRoamMode)ROAM_MODE;
    if (order->data.roam.mode == BTECH_AUTOPILOT_ROAM_MAP) {
      check_order_options(state, 2, ROAM_MAP_FIELDS, 2);
    } else {
      check_order_options(state, 2, ROAM_RADIUS_FIELDS, 5);
      order->data.roam.x =
          (int)lua_btech_check_integer_field(state, 2, "x", 0, INT_MAX, 2);
      order->data.roam.y =
          (int)lua_btech_check_integer_field(state, 2, "y", 0, INT_MAX, 2);
      order->data.roam.radius =
          (int)lua_btech_check_integer_field(state, 2, "radius", 1, 30, 2);
    }
    break;
  case BTECH_AUTOPILOT_ORDER_AUTOGUN:
    const int AUTOGUN_MODE = require_constant_field(state, package, 2, "mode",
                                                    &AUTOGUN_MODE_CATALOG);
    order->data.autogun.mode = (BtechAutopilotAutogunMode)AUTOGUN_MODE;
    if (order->data.autogun.mode == BTECH_AUTOPILOT_AUTOGUN_TARGET) {
      check_order_options(state, 2, AUTOGUN_TARGET_FIELDS, 3);
      order->data.autogun.target =
          require_unit_field(state, package, 2, "target");
    } else {
      check_order_options(state, 2, AUTOGUN_FIELDS, 2);
    }
    break;
  case BTECH_AUTOPILOT_ORDER_DROPOFF:
  case BTECH_AUTOPILOT_ORDER_SHUTDOWN:
  case BTECH_AUTOPILOT_ORDER_STARTUP:
  case BTECH_AUTOPILOT_ORDER_UDISEMBARK:
    check_order_options(state, 2, NAME_FIELDS, 1);
    break;
  case BTECH_AUTOPILOT_ORDER_SPEED:
    check_order_options(state, 2, SPEED_FIELDS, 2);
    order->data.speed.percent =
        (int)lua_btech_check_integer_field(state, 2, "percent", 1, 100, 2);
    break;
  }
}

static int result_error(lua_State *state, BtechAutopilotResult result) {
  switch (result) {
  case BTECH_AUTOPILOT_INVALID_ARGUMENT:
    return lua_error_raise(state, LUA_ERROR_CODE_ARG_INVALID,
                           "invalid autopilot argument");
  case BTECH_AUTOPILOT_UNAVAILABLE:
    return lua_error_raise(state, LUA_ERROR_CODE_OBJECT_UNAVAILABLE,
                           "autopilot runtime state is unavailable");
  case BTECH_AUTOPILOT_NO_MEMORY:
    return lua_error_raise(state, LUA_ERROR_CODE_INTERNAL,
                           "unable to allocate autopilot state");
  case BTECH_AUTOPILOT_NOT_INSTALLED_IN_UNIT:
    return lua_btech_operation_error(state, "not_installed_in_unit",
                                     "autopilot is not installed in a unit");
  case BTECH_AUTOPILOT_ALREADY_ENGAGED:
    return lua_btech_operation_error(state, "already_engaged",
                                     "autopilot is already engaged");
  case BTECH_AUTOPILOT_ALREADY_DISENGAGED:
    return lua_btech_operation_error(state, "already_disengaged",
                                     "autopilot is already disengaged");
  case BTECH_AUTOPILOT_CONFLICT:
    return lua_btech_operation_error(state, "association_conflict",
                                     "autopilot association conflicts");
  case BTECH_AUTOPILOT_ACTIVE_ORDER:
    return lua_btech_operation_error(state, "active_order",
                                     "the active order cannot be removed");
  case BTECH_AUTOPILOT_QUEUE_FULL:
    return lua_btech_operation_error(state, "queue_full",
                                     "the autopilot order queue is full");
  case BTECH_AUTOPILOT_ORDER_NOT_FOUND:
    return lua_btech_operation_error(state, "order_not_found",
                                     "the requested order does not exist");
  case BTECH_AUTOPILOT_INVALID_ORDER_QUEUE:
    return lua_btech_operation_error(state, "invalid_order_queue",
                                     "the stored order queue is invalid");
  case BTECH_AUTOPILOT_OK:
    break;
  }
  return lua_error_raise(state, LUA_ERROR_CODE_INTERNAL,
                         "unknown autopilot operation result");
}

static const char *association_name(BtechAutopilotAssociation association) {
  switch (association) {
  case BTECH_AUTOPILOT_ASSOCIATION_NONE:
    return "none";
  case BTECH_AUTOPILOT_ASSOCIATION_READY:
    return "ready";
  case BTECH_AUTOPILOT_ASSOCIATION_ASSOCIATED:
    return "associated";
  case BTECH_AUTOPILOT_ASSOCIATION_CONFLICT:
    return "conflict";
  }
  return "conflict";
}

static void set_integer(lua_State *state, const char *field,
                        lua_Integer value) {
  lua_pushinteger(state, value);
  lua_setfield(state, -2, field);
}

static void set_boolean(lua_State *state, const char *field, bool value) {
  lua_pushboolean(state, (int)value);
  lua_setfield(state, -2, field);
}

static void push_order(lua_State *state, LuaBtechPackage *package,
                       const BtechAutopilotOrder *order) {
  lua_newtable(state);
  lua_btech_constant_push(state, package, &ORDER_CATALOG, order->type);
  lua_setfield(state, -2, "name");
  switch (order->type) {
  case BTECH_AUTOPILOT_ORDER_CHASETARGET:
  case BTECH_AUTOPILOT_ORDER_DUMBFOLLOW:
  case BTECH_AUTOPILOT_ORDER_FOLLOW:
  case BTECH_AUTOPILOT_ORDER_EMBARK:
  case BTECH_AUTOPILOT_ORDER_PICKUP:
    lua_btech_push_object(state, package, (DbRef)order->data.target.target);
    lua_setfield(state, -2, "target");
    break;
  case BTECH_AUTOPILOT_ORDER_DUMBGOTO:
  case BTECH_AUTOPILOT_ORDER_GOTO:
  case BTECH_AUTOPILOT_ORDER_OLDGOTO:
    set_integer(state, "x", order->data.destination.x);
    set_integer(state, "y", order->data.destination.y);
    break;
  case BTECH_AUTOPILOT_ORDER_ENTERBASE: {
    lua_btech_constant_push(state, package, &DIRECTION_CATALOG,
                            order->data.enter_base.direction);
    lua_setfield(state, -2, "direction");
    break;
  }
  case BTECH_AUTOPILOT_ORDER_LEAVEBASE:
    set_integer(state, "heading", order->data.leave_base.heading);
    break;
  case BTECH_AUTOPILOT_ORDER_ROAM:
    lua_btech_constant_push(state, package, &ROAM_MODE_CATALOG,
                            order->data.roam.mode);
    lua_setfield(state, -2, "mode");
    if (order->data.roam.mode == BTECH_AUTOPILOT_ROAM_RADIUS) {
      set_integer(state, "x", order->data.roam.x);
      set_integer(state, "y", order->data.roam.y);
      set_integer(state, "radius", order->data.roam.radius);
    }
    break;
  case BTECH_AUTOPILOT_ORDER_AUTOGUN:
    lua_btech_constant_push(state, package, &AUTOGUN_MODE_CATALOG,
                            order->data.autogun.mode);
    lua_setfield(state, -2, "mode");
    if (order->data.autogun.mode == BTECH_AUTOPILOT_AUTOGUN_TARGET) {
      lua_btech_push_object(state, package, (DbRef)order->data.autogun.target);
      lua_setfield(state, -2, "target");
    }
    break;
  case BTECH_AUTOPILOT_ORDER_SPEED:
    set_integer(state, "percent", order->data.speed.percent);
    break;
  case BTECH_AUTOPILOT_ORDER_DROPOFF:
  case BTECH_AUTOPILOT_ORDER_SHUTDOWN:
  case BTECH_AUTOPILOT_ORDER_STARTUP:
  case BTECH_AUTOPILOT_ORDER_UDISEMBARK:
    break;
  }
}

static void set_optional_object(lua_State *state, LuaBtechPackage *package,
                                const char *field, BtechObjectId object) {
  lua_btech_push_optional_object(state, package, (DbRef)object);
  lua_setfield(state, -2, field);
}

static int lua_autopilot_status(lua_State *state, LuaBtechPackage *package) {
  const DbRef AUTOPILOT = require_autopilot(state, package);
  BtechAutopilotStatus status;
  const BtechAutopilotResult RESULT = btech_autopilot_status(
      lua_btech_context(package), (BtechObjectId)AUTOPILOT, &status);
  if (RESULT != BTECH_AUTOPILOT_OK)
    return result_error(state, RESULT);
  lua_newtable(state);
  set_optional_object(state, package, "container_unit", status.container_unit);
  set_optional_object(state, package, "associated_unit",
                      status.associated_unit);
  set_optional_object(state, package, "map", status.map);
  lua_pushstring(state, association_name(status.association));
  lua_setfield(state, -2, "association");
  set_boolean(state, "engaged", status.engaged);
  set_integer(state, "speed_percent", status.speed_percent);
  set_integer(state, "order_count", (lua_Integer)status.order_count);
  set_integer(state, "order_capacity", (lua_Integer)status.order_capacity);
  lua_newtable(state);
  for (size_t index = 0; index < status.order_count; index++) {
    const BtechAutopilotOrder *order =
        checked_storage_at_const(status.orders, BTECH_AUTOPILOT_ORDER_CAPACITY,
                                 sizeof(*status.orders), index);
    push_order(state, package, order);
    lua_rawseti(state, -2, (int)index + 1);
  }
  lua_setfield(state, -2, "orders");
  lua_newtable(state);
  set_boolean(state, "autogun_enabled", status.modes.autogun_enabled);
  set_boolean(state, "autogun_suspended", status.modes.autogun_suspended);
  set_boolean(state, "piloting_suspended", status.modes.piloting_suspended);
  set_boolean(state, "roaming", status.modes.roaming);
  set_boolean(state, "manual_sensors", status.modes.manual_sensors);
  set_boolean(state, "chasing_target", status.modes.chasing_target);
  set_boolean(state, "chase_resume_pending", status.modes.chase_resume_pending);
  set_boolean(state, "swarm_charging", status.modes.swarm_charging);
  set_boolean(state, "assigned_target", status.modes.assigned_target);
  lua_setfield(state, -2, "modes");
  return 1;
}

static int lua_add_order(lua_State *state, LuaBtechPackage *package) {
  const DbRef AUTOPILOT = require_autopilot(state, package);
  lua_btech_check_arity(state, 2);
  BtechAutopilotOrder order;
  parse_order(state, package, &order);
  const BtechAutopilotResult RESULT = btech_autopilot_add_order(
      lua_btech_context(package), (BtechObjectId)AUTOPILOT, &order);
  return RESULT == BTECH_AUTOPILOT_OK ? 0 : result_error(state, RESULT);
}

static int lua_remove_order(lua_State *state, LuaBtechPackage *package) {
  const DbRef AUTOPILOT = require_autopilot(state, package);
  lua_btech_check_arity(state, 2);
  if (lua_type(state, 2) != LUA_TNUMBER)
    (void)lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                        "position must be an integer");
  const lua_Number NUMBER = lua_tonumber(state, 2);
  if (!isfinite(NUMBER) || NUMBER < 1 ||
      NUMBER > (lua_Number)BTECH_AUTOPILOT_ORDER_CAPACITY ||
      floor(NUMBER) != NUMBER)
    (void)lua_error_arg(state, 2, LUA_ERROR_CODE_ARG_INVALID,
                        "position must be an integer from 1 to 100");
  const BtechAutopilotResult RESULT = btech_autopilot_remove_order(
      lua_btech_context(package), (BtechObjectId)AUTOPILOT, (size_t)NUMBER);
  return RESULT == BTECH_AUTOPILOT_OK ? 0 : result_error(state, RESULT);
}

static int lua_clear_orders(lua_State *state, LuaBtechPackage *package) {
  const DbRef AUTOPILOT = require_autopilot(state, package);
  const BtechAutopilotResult RESULT = btech_autopilot_clear_orders(
      lua_btech_context(package), (BtechObjectId)AUTOPILOT);
  return RESULT == BTECH_AUTOPILOT_OK ? 0 : result_error(state, RESULT);
}

static int lua_engage(lua_State *state, LuaBtechPackage *package) {
  const DbRef AUTOPILOT = require_autopilot(state, package);
  const BtechAutopilotResult RESULT = btech_autopilot_engage(
      lua_btech_context(package), (BtechObjectId)AUTOPILOT);
  return RESULT == BTECH_AUTOPILOT_OK ? 0 : result_error(state, RESULT);
}

static int lua_disengage(lua_State *state, LuaBtechPackage *package) {
  const DbRef AUTOPILOT = require_autopilot(state, package);
  const BtechAutopilotResult RESULT = btech_autopilot_disengage(
      lua_btech_context(package), (BtechObjectId)AUTOPILOT);
  return RESULT == BTECH_AUTOPILOT_OK ? 0 : result_error(state, RESULT);
}

static int lua_event_stats(lua_State *state, LuaBtechPackage *package) {
  const DbRef AUTOPILOT = require_autopilot(state, package);
  BtechAutopilotEventStats stats;
  const BtechAutopilotResult RESULT = btech_autopilot_event_stats(
      lua_btech_context(package), (BtechObjectId)AUTOPILOT, &stats);
  if (RESULT != BTECH_AUTOPILOT_OK)
    return result_error(state, RESULT);
  lua_newtable(state);
  set_integer(state, "goto", (lua_Integer)stats.goto_count);
  set_integer(state, "leave", (lua_Integer)stats.leave_count);
  set_integer(state, "command", (lua_Integer)stats.command_count);
  set_integer(state, "autogun", (lua_Integer)stats.autogun_count);
  set_integer(state, "sensor", (lua_Integer)stats.sensor_count);
  set_integer(state, "follow", (lua_Integer)stats.follow_count);
  set_integer(state, "enter_base", (lua_Integer)stats.enter_base_count);
  set_integer(state, "reply", (lua_Integer)stats.reply_count);
  set_integer(state, "profile", (lua_Integer)stats.profile_count);
  set_integer(state, "roam", (lua_Integer)stats.roam_count);
  set_integer(state, "total", (lua_Integer)stats.total);
  return 1;
}

void lua_btech_install_autopilot_bindings(lua_State *state,
                                          LuaBtechPackage *package) {
  static const BtechLuaNativeEntry ENTRIES[] = {
      {"status", "autopilot.status", lua_autopilot_status},
      {"add_order", "autopilot.add_order", lua_add_order},
      {"remove_order", "autopilot.remove_order", lua_remove_order},
      {"clear_orders", "autopilot.clear_orders", lua_clear_orders},
      {"engage", "autopilot.engage", lua_engage},
      {"disengage", "autopilot.disengage", lua_disengage},
      {"event_stats", "autopilot.event_stats", lua_event_stats},
  };
  lua_btech_install_native_bindings(state, package, "autopilot", ENTRIES,
                                    sizeof(ENTRIES) / sizeof(*ENTRIES));
  lua_getfield(state, -1, "autopilot");
  lua_btech_constant_install_namespace(state, package, "orders",
                                       &ORDER_CATALOG);
  lua_btech_constant_install_namespace(state, package, "directions",
                                       &DIRECTION_CATALOG);
  lua_btech_constant_install_namespace(state, package, "roam_modes",
                                       &ROAM_MODE_CATALOG);
  lua_btech_constant_install_namespace(state, package, "autogun_modes",
                                       &AUTOGUN_MODE_CATALOG);
  lua_pop(state, 1);
}
