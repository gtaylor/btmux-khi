/** @file
 * Typed, notification-free administration of BattleTech autopilots.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "btech/ids.h"

typedef struct BtechContext BtechContext;

/** Maximum number of orders retained by an autopilot. */
enum { BTECH_AUTOPILOT_ORDER_CAPACITY = 100 };

/** Stable result of an autopilot operation. */
typedef enum BtechAutopilotResult : int {
  BTECH_AUTOPILOT_OK,
  BTECH_AUTOPILOT_INVALID_ARGUMENT,
  BTECH_AUTOPILOT_UNAVAILABLE,
  BTECH_AUTOPILOT_NOT_INSTALLED_IN_UNIT,
  BTECH_AUTOPILOT_ALREADY_ENGAGED,
  BTECH_AUTOPILOT_ALREADY_DISENGAGED,
  BTECH_AUTOPILOT_CONFLICT,
  BTECH_AUTOPILOT_ACTIVE_ORDER,
  BTECH_AUTOPILOT_QUEUE_FULL,
  BTECH_AUTOPILOT_ORDER_NOT_FOUND,
  BTECH_AUTOPILOT_INVALID_ORDER_QUEUE,
  BTECH_AUTOPILOT_NO_MEMORY,
} BtechAutopilotResult;

/** Supported typed autopilot order. */
typedef enum BtechAutopilotOrderType : int {
  BTECH_AUTOPILOT_ORDER_CHASETARGET,
  BTECH_AUTOPILOT_ORDER_DUMBFOLLOW,
  BTECH_AUTOPILOT_ORDER_DUMBGOTO,
  BTECH_AUTOPILOT_ORDER_ENTERBASE,
  BTECH_AUTOPILOT_ORDER_FOLLOW,
  BTECH_AUTOPILOT_ORDER_GOTO,
  BTECH_AUTOPILOT_ORDER_LEAVEBASE,
  BTECH_AUTOPILOT_ORDER_OLDGOTO,
  BTECH_AUTOPILOT_ORDER_ROAM,
  BTECH_AUTOPILOT_ORDER_AUTOGUN,
  BTECH_AUTOPILOT_ORDER_DROPOFF,
  BTECH_AUTOPILOT_ORDER_EMBARK,
  BTECH_AUTOPILOT_ORDER_PICKUP,
  BTECH_AUTOPILOT_ORDER_SHUTDOWN,
  BTECH_AUTOPILOT_ORDER_SPEED,
  BTECH_AUTOPILOT_ORDER_STARTUP,
  BTECH_AUTOPILOT_ORDER_UDISEMBARK,
} BtechAutopilotOrderType;

/** Cardinal direction used by ENTERBASE. */
typedef enum BtechAutopilotDirection : int {
  BTECH_AUTOPILOT_NORTH,
  BTECH_AUTOPILOT_EAST,
  BTECH_AUTOPILOT_SOUTH,
  BTECH_AUTOPILOT_WEST,
} BtechAutopilotDirection;

/** Region selected by ROAM. */
typedef enum BtechAutopilotRoamMode : int {
  BTECH_AUTOPILOT_ROAM_MAP,
  BTECH_AUTOPILOT_ROAM_RADIUS,
} BtechAutopilotRoamMode;

/** Target-selection mode selected by AUTOGUN. */
typedef enum BtechAutopilotAutogunMode : int {
  BTECH_AUTOPILOT_AUTOGUN_AUTOMATIC,
  BTECH_AUTOPILOT_AUTOGUN_OFF,
  BTECH_AUTOPILOT_AUTOGUN_TARGET,
} BtechAutopilotAutogunMode;

/** One validated autopilot order. Only the member selected by type is used. */
typedef struct BtechAutopilotOrder {
  BtechAutopilotOrderType type;
  union {
    struct {
      BtechObjectId target;
    } target;
    struct {
      int x;
      int y;
    } destination;
    struct {
      BtechAutopilotDirection direction;
    } enter_base;
    struct {
      int heading;
    } leave_base;
    struct {
      BtechAutopilotRoamMode mode;
      int x;
      int y;
      int radius;
    } roam;
    struct {
      BtechAutopilotAutogunMode mode;
      BtechObjectId target;
    } autogun;
    struct {
      int percent;
    } speed;
  } data;
} BtechAutopilotOrder;

/** Relationship between physical containment and cached association. */
typedef enum BtechAutopilotAssociation : int {
  BTECH_AUTOPILOT_ASSOCIATION_NONE,
  BTECH_AUTOPILOT_ASSOCIATION_READY,
  BTECH_AUTOPILOT_ASSOCIATION_ASSOCIATED,
  BTECH_AUTOPILOT_ASSOCIATION_CONFLICT,
} BtechAutopilotAssociation;

/** Runtime modes reported for an autopilot. */
typedef struct BtechAutopilotModes {
  bool autogun_enabled;
  bool autogun_suspended;
  bool piloting_suspended;
  bool roaming;
  bool manual_sensors;
  bool chasing_target;
  bool chase_resume_pending;
  bool swarm_charging;
  bool assigned_target;
} BtechAutopilotModes;

/** Complete snapshot of one autopilot and its queue. */
typedef struct BtechAutopilotStatus {
  BtechObjectId container_unit;
  BtechObjectId associated_unit;
  BtechObjectId map;
  BtechAutopilotAssociation association;
  bool engaged;
  int speed_percent;
  size_t order_count;
  size_t order_capacity;
  BtechAutopilotOrder orders[BTECH_AUTOPILOT_ORDER_CAPACITY];
  BtechAutopilotModes modes;
} BtechAutopilotStatus;

/** Counts of pending events associated with one autopilot. */
typedef struct BtechAutopilotEventStats {
  size_t goto_count;
  size_t leave_count;
  size_t command_count;
  size_t autogun_count;
  size_t sensor_count;
  size_t follow_count;
  size_t enter_base_count;
  size_t reply_count;
  size_t profile_count;
  size_t roam_count;
  size_t total;
} BtechAutopilotEventStats;

/** Validates immutable order shape, scalar bounds, and registered targets. */
BtechAutopilotResult
btech_autopilot_order_validate(BtechContext *context,
                               const BtechAutopilotOrder *order);

/** Returns a fresh status snapshot, rejecting malformed stored orders. */
BtechAutopilotResult btech_autopilot_status(BtechContext *context,
                                            BtechObjectId autopilot,
                                            BtechAutopilotStatus *status);

/** Appends one typed order atomically and rearms an engaged idle queue. */
BtechAutopilotResult
btech_autopilot_add_order(BtechContext *context, BtechObjectId autopilot,
                          const BtechAutopilotOrder *order);

/** Removes a one-based order position, protecting an engaged active head. */
BtechAutopilotResult btech_autopilot_remove_order(BtechContext *context,
                                                  BtechObjectId autopilot,
                                                  size_t position);

/** Clears the queue unless a nonempty queue is currently engaged. */
BtechAutopilotResult btech_autopilot_clear_orders(BtechContext *context,
                                                  BtechObjectId autopilot);

/** Safely associates and engages an autopilot physically contained in a unit.
 */
BtechAutopilotResult btech_autopilot_engage(BtechContext *context,
                                            BtechObjectId autopilot);

/** Stops an engaged autopilot while preserving association and queued orders.
 */
BtechAutopilotResult btech_autopilot_disengage(BtechContext *context,
                                               BtechObjectId autopilot);

/** Returns pending event counts, including explicit zero-valued categories. */
BtechAutopilotResult
btech_autopilot_event_stats(BtechContext *context, BtechObjectId autopilot,
                            BtechAutopilotEventStats *stats);
