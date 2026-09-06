/* Implements notification-free typed autopilot administration. */

#include "autopilot_control_api.h"

#include <stddef.h>

#include "autopilot.h"
#include "autopilot_argument_list_api.h"
#include "autopilot_order_codec_api.h"
#include "autopilot_order_queue_api.h"
#include "btech/autopilot.h"
#include "btech/context.h"
#include "btech/ids.h"
#include "btech_event.h"
#include "context_internal.h" // IWYU pragma: keep
#include "mech_api_types.h"
#include "mech_events.h"
#include "mech_identity_api.h"
#include "mech_runtime_api.h"
#include "mux/network/mux_event.h"
#include "mux/objects/db.h"
#include "mux/support/checked_storage.h"
#include "registry_api.h"

static Autopilot *find_autopilot(BtechContext *context,
                                 BtechObjectId autopilot) {
  if (context == nullptr || !btech_context_is_auto(context, autopilot))
    return nullptr;
  return btech_context_find_object(context, autopilot);
}

static size_t event_count(MuxEventScheduler *events, int type,
                          Autopilot *autopilot) {
  const int COUNT = mux_event_count_type_data(events, type, autopilot);
  return COUNT > 0 ? (size_t)COUNT : 0;
}

static bool valid_claim(const Autopilot *autopilot, Mech *unit,
                        BtechObjectId claimant) {
  if (claimant <= 0 || claimant == autopilot->mynum)
    return false;
  BtechContext *context = autopilot->xcode.context;
  Autopilot *other = find_autopilot(context, claimant);
  return (other != nullptr &&
          game_object_location(btech_context_database(context), claimant) ==
              mech_dbref(unit)) != 0;
}

AutopilotAssociationStatus
autopilot_control_association(const Autopilot *autopilot) {
  AutopilotAssociationStatus result = {
      .association = BTECH_AUTOPILOT_ASSOCIATION_NONE,
      .container = -1,
      .associated = -1,
  };
  if (autopilot == nullptr)
    return result;

  BtechContext *context = autopilot->xcode.context;
  const BtechObjectId LOCATION =
      game_object_location(btech_context_database(context), autopilot->mynum);
  Mech *contained = btech_context_get_mech(context, LOCATION);
  Mech *cached = btech_context_get_mech(context, autopilot->mymechnum);
  if (contained != nullptr)
    result.container = LOCATION;
  if (cached != nullptr)
    result.associated = autopilot->mymechnum;

  if (cached != nullptr && autopilot->mymechnum != LOCATION &&
      mech_autopilot_dbref(cached) == autopilot->mynum) {
    result.association = BTECH_AUTOPILOT_ASSOCIATION_CONFLICT;
    return result;
  }
  if (contained == nullptr) {
    if (cached != nullptr)
      result.association = BTECH_AUTOPILOT_ASSOCIATION_CONFLICT;
    return result;
  }
  const BtechObjectId CLAIMANT = mech_autopilot_dbref(contained);
  if (valid_claim(autopilot, contained, CLAIMANT)) {
    result.association = BTECH_AUTOPILOT_ASSOCIATION_CONFLICT;
    return result;
  }
  result.association =
      autopilot->mymechnum == LOCATION && CLAIMANT == autopilot->mynum
          ? BTECH_AUTOPILOT_ASSOCIATION_ASSOCIATED
          : BTECH_AUTOPILOT_ASSOCIATION_READY;
  return result;
}

static void schedule_command_if_needed(Autopilot *autopilot) {
  if (mux_event_count_type_data(autopilot->xcode.context->events, EVENT_AUTOCOM,
                                autopilot) == 0)
    autopilot_event_schedule(autopilot, EVENT_AUTOCOM, auto_com_event,
                             AUTOPILOT_NC_DELAY, 0);
}

BtechAutopilotResult autopilot_control_engage(Autopilot *autopilot) {
  if (autopilot == nullptr)
    return BTECH_AUTOPILOT_UNAVAILABLE;
  if (autopilot->engaged)
    return BTECH_AUTOPILOT_ALREADY_ENGAGED;

  const AutopilotAssociationStatus ASSOCIATION =
      autopilot_control_association(autopilot);
  if (ASSOCIATION.container < 0)
    return BTECH_AUTOPILOT_NOT_INSTALLED_IN_UNIT;
  if (ASSOCIATION.association == BTECH_AUTOPILOT_ASSOCIATION_CONFLICT)
    return BTECH_AUTOPILOT_CONFLICT;

  Mech *mech =
      btech_context_get_mech(autopilot->xcode.context, ASSOCIATION.container);
  autopilot->mymech = mech;
  autopilot->mymechnum = ASSOCIATION.container;
  mech_autopilot_dbref_set(mech, autopilot->mynum);
  autopilot->mapindex = mech_map_dbref(mech);
  autopilot->engaged = true;
  auto_set_comtitle(autopilot, mech);
  if (autopilot_order_count(autopilot) > 0)
    schedule_command_if_needed(autopilot);
  return BTECH_AUTOPILOT_OK;
}

BtechAutopilotResult autopilot_control_disengage(Autopilot *autopilot) {
  if (autopilot == nullptr)
    return BTECH_AUTOPILOT_UNAVAILABLE;
  if (!autopilot->engaged)
    return BTECH_AUTOPILOT_ALREADY_DISENGAGED;
  auto_stop_pilot(autopilot);
  return BTECH_AUTOPILOT_OK;
}

static BtechAutopilotResult queue_result(AutopilotOrderResult result) {
  switch (result) {
  case AUTOPILOT_ORDER_OK:
    return BTECH_AUTOPILOT_OK;
  case AUTOPILOT_ORDER_FULL:
    return BTECH_AUTOPILOT_QUEUE_FULL;
  case AUTOPILOT_ORDER_NOT_FOUND:
    return BTECH_AUTOPILOT_ORDER_NOT_FOUND;
  case AUTOPILOT_ORDER_NO_MEMORY:
    return BTECH_AUTOPILOT_NO_MEMORY;
  case AUTOPILOT_ORDER_INVALID:
  case AUTOPILOT_ORDER_UNSUPPORTED:
    return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  }
  return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
}

BtechAutopilotResult
autopilot_control_enqueue(Autopilot *autopilot,
                          const AutopilotCommandDefinition *definition,
                          const AutopilotArgumentList *arguments) {
  if (autopilot == nullptr)
    return BTECH_AUTOPILOT_UNAVAILABLE;
  const size_t OLD_COUNT = autopilot_order_count(autopilot);
  const BtechAutopilotResult RESULT =
      queue_result(autopilot_order_enqueue(autopilot, definition, arguments));
  if (RESULT == BTECH_AUTOPILOT_OK && autopilot->engaged && OLD_COUNT == 0)
    schedule_command_if_needed(autopilot);
  return RESULT;
}

BtechAutopilotResult autopilot_control_remove(Autopilot *autopilot,
                                              size_t position) {
  if (autopilot == nullptr)
    return BTECH_AUTOPILOT_UNAVAILABLE;
  if (position == 0 || position > autopilot_order_count(autopilot))
    return BTECH_AUTOPILOT_ORDER_NOT_FOUND;
  if (autopilot->engaged && position == 1)
    return BTECH_AUTOPILOT_ACTIVE_ORDER;
  return queue_result(autopilot_order_remove(autopilot, position - 1));
}

BtechAutopilotResult autopilot_control_clear(Autopilot *autopilot) {
  if (autopilot == nullptr)
    return BTECH_AUTOPILOT_UNAVAILABLE;
  if (autopilot->engaged && autopilot_order_count(autopilot) > 0)
    return BTECH_AUTOPILOT_ACTIVE_ORDER;
  autopilot_order_clear(autopilot);
  return BTECH_AUTOPILOT_OK;
}

BtechAutopilotResult btech_autopilot_status(BtechContext *context,
                                            BtechObjectId autopilot_dbref,
                                            BtechAutopilotStatus *status) {
  Autopilot *autopilot = find_autopilot(context, autopilot_dbref);
  if (autopilot == nullptr)
    return BTECH_AUTOPILOT_UNAVAILABLE;
  if (status == nullptr)
    return BTECH_AUTOPILOT_INVALID_ARGUMENT;
  BtechAutopilotStatus value = {0};
  value.map = -1;
  if (autopilot->mapindex >= 0 &&
      btech_context_get_map(context, autopilot->mapindex) != nullptr)
    value.map = autopilot->mapindex;
  const AutopilotAssociationStatus ASSOCIATION =
      autopilot_control_association(autopilot);
  value.container_unit = ASSOCIATION.container;
  value.associated_unit = ASSOCIATION.associated;
  value.association = ASSOCIATION.association;
  value.engaged = autopilot->engaged;
  value.speed_percent = autopilot->speed;
  value.order_count = autopilot_order_count(autopilot);
  value.order_capacity = BTECH_AUTOPILOT_ORDER_CAPACITY;
  if (value.order_count > value.order_capacity)
    return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  value.modes = (BtechAutopilotModes){
      .autogun_enabled = (autopilot->flags & AUTOPILOT_AUTOGUN) != 0,
      .autogun_suspended = (autopilot->flags & AUTOPILOT_GUNZOMBIE) != 0,
      .piloting_suspended = (autopilot->flags & AUTOPILOT_PILZOMBIE) != 0,
      .roaming = (autopilot->flags & AUTOPILOT_ROAM) != 0,
      .manual_sensors = (autopilot->flags & AUTOPILOT_LSENS) != 0,
      .chasing_target = (autopilot->flags & AUTOPILOT_CHASETARG) != 0,
      .chase_resume_pending = (autopilot->flags & AUTOPILOT_WAS_CHASE_ON) != 0,
      .swarm_charging = (autopilot->flags & AUTOPILOT_SWARMCHARGE) != 0,
      .assigned_target = (autopilot->flags & AUTOPILOT_ASSIGNED_TARGET) != 0,
  };
  for (size_t index = 0; index < value.order_count; index++) {
    BtechAutopilotOrder *output =
        checked_storage_at(value.orders, BTECH_AUTOPILOT_ORDER_CAPACITY,
                           sizeof(BtechAutopilotOrder), index);
    const BtechAutopilotResult RESULT =
        autopilot_order_decode(autopilot_order_at(autopilot, index), output);
    if (RESULT != BTECH_AUTOPILOT_OK)
      return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
    if (btech_autopilot_order_validate(context, output) != BTECH_AUTOPILOT_OK)
      return BTECH_AUTOPILOT_INVALID_ORDER_QUEUE;
  }
  *status = value;
  return BTECH_AUTOPILOT_OK;
}

BtechAutopilotResult
btech_autopilot_add_order(BtechContext *context, BtechObjectId autopilot_dbref,
                          const BtechAutopilotOrder *order) {
  Autopilot *autopilot = find_autopilot(context, autopilot_dbref);
  if (autopilot == nullptr)
    return BTECH_AUTOPILOT_UNAVAILABLE;
  const AutopilotCommandDefinition *definition = nullptr;
  AutopilotArgumentList arguments = {};
  BtechAutopilotResult result =
      autopilot_order_encode(context, order, &definition, &arguments);
  if (result == BTECH_AUTOPILOT_OK)
    result = autopilot_control_enqueue(autopilot, definition, &arguments);
  autopilot_argument_list_destroy(&arguments);
  return result;
}

BtechAutopilotResult btech_autopilot_remove_order(BtechContext *context,
                                                  BtechObjectId autopilot_dbref,
                                                  size_t position) {
  return autopilot_control_remove(find_autopilot(context, autopilot_dbref),
                                  position);
}

BtechAutopilotResult
btech_autopilot_clear_orders(BtechContext *context,
                             BtechObjectId autopilot_dbref) {
  return autopilot_control_clear(find_autopilot(context, autopilot_dbref));
}

BtechAutopilotResult btech_autopilot_engage(BtechContext *context,
                                            BtechObjectId autopilot_dbref) {
  return autopilot_control_engage(find_autopilot(context, autopilot_dbref));
}

BtechAutopilotResult btech_autopilot_disengage(BtechContext *context,
                                               BtechObjectId autopilot_dbref) {
  return autopilot_control_disengage(find_autopilot(context, autopilot_dbref));
}

BtechAutopilotResult
btech_autopilot_event_stats(BtechContext *context,
                            BtechObjectId autopilot_dbref,
                            BtechAutopilotEventStats *stats) {
  Autopilot *autopilot = find_autopilot(context, autopilot_dbref);
  if (autopilot == nullptr)
    return BTECH_AUTOPILOT_UNAVAILABLE;
  if (stats == nullptr)
    return BTECH_AUTOPILOT_INVALID_ARGUMENT;
  MuxEventScheduler *events = autopilot->xcode.context->events;
  BtechAutopilotEventStats value = {
      .goto_count = event_count(events, EVENT_AUTOGOTO, autopilot),
      .leave_count = event_count(events, EVENT_AUTOLEAVE, autopilot),
      .command_count = event_count(events, EVENT_AUTOCOM, autopilot),
      .autogun_count = event_count(events, EVENT_AUTOGUN, autopilot),
      .sensor_count = event_count(events, EVENT_AUTO_SENSOR, autopilot),
      .follow_count = event_count(events, EVENT_AUTOFOLLOW, autopilot),
      .enter_base_count = event_count(events, EVENT_AUTOENTERBASE, autopilot),
      .reply_count = event_count(events, EVENT_AUTO_REPLY, autopilot),
      .profile_count = event_count(events, EVENT_AUTO_PROFILE, autopilot),
      .roam_count = event_count(events, EVENT_AUTO_ROAM, autopilot),
  };
  value.total = value.goto_count + value.leave_count + value.command_count +
                value.autogun_count + value.sensor_count + value.follow_count +
                value.enter_base_count + value.reply_count +
                value.profile_count + value.roam_count;
  *stats = value;
  return BTECH_AUTOPILOT_OK;
}
