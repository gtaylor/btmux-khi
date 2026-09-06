#include <string.h>

#include "autopilot.h"
#include "autopilot_order_codec_api.h"
#include "autopilot_order_queue_api.h"
#include "btech/autopilot.h"
#include "btech/context.h"
#include "context_internal.h"
#include "mech_identity_api.h"
#include "mech_runtime_api.h"
#include "mux/network/mux_event.h"
#include "mux/objects/db.h"
#include "registry_api.h"

static Autopilot *fixture_autopilot;
static DbRef fixture_map = -1;
static size_t fixture_order_count;
static BtechAutopilotResult fixture_validation_result = BTECH_AUTOPILOT_OK;

bool btech_context_is_auto(BtechContext *context, DbRef key) {
  return context != nullptr && fixture_autopilot != nullptr &&
         key == fixture_autopilot->mynum;
}

void *btech_context_find_object(BtechContext *context, DbRef key) {
  return btech_context_is_auto(context, key) ? fixture_autopilot : nullptr;
}

GameDatabase *btech_context_database(BtechContext *context) {
  return context->database;
}

Mech *btech_context_get_mech(BtechContext *context [[maybe_unused]],
                             DbRef key [[maybe_unused]]) {
  return nullptr;
}

BattleMap *btech_context_get_map(BtechContext *context, DbRef key) {
  return key == fixture_map ? (BattleMap *)context : nullptr;
}

DbRef mech_autopilot_dbref(const Mech *mech [[maybe_unused]]) { return -1; }

DbRef mech_dbref(const Mech *mech [[maybe_unused]]) { return -1; }

size_t autopilot_order_count(const Autopilot *autopilot [[maybe_unused]]) {
  return fixture_order_count;
}

const AutopilotCommand *autopilot_order_at(const Autopilot *autopilot
                                           [[maybe_unused]],
                                           size_t index [[maybe_unused]]) {
  return (const AutopilotCommand *)fixture_autopilot;
}

BtechAutopilotResult autopilot_order_decode(const AutopilotCommand *command
                                            [[maybe_unused]],
                                            BtechAutopilotOrder *order) {
  *order = (BtechAutopilotOrder){.type = BTECH_AUTOPILOT_ORDER_FOLLOW,
                                 .data.target.target = 999999999};
  return BTECH_AUTOPILOT_OK;
}

BtechAutopilotResult
btech_autopilot_order_validate(BtechContext *context [[maybe_unused]],
                               const BtechAutopilotOrder *order
                               [[maybe_unused]]) {
  return fixture_validation_result;
}

int mux_event_count_type_data(MuxEventScheduler *scheduler [[maybe_unused]],
                              int type, const void *data) {
  if (data != fixture_autopilot)
    return 0;
  if (type == EVENT_AUTOGOTO)
    return 2;
  if (type == EVENT_AUTOFOLLOW)
    return 3;
  if (type == EVENT_AUTO_ROAM)
    return 4;
  return 0;
}

int main(void) {
  GameObject objects[2] = {{.location = -1}, {.location = -1}};
  GameDatabase database = {.object_storage = objects, .top = 1, .size = 1};
  MuxEventScheduler *events = (MuxEventScheduler *)objects;
  BtechContext context = {.database = &database, .events = events};
  Autopilot autopilot = {
      .xcode = {.type = GTYPE_AUTO, .context = &context},
      .mynum = 0,
      .mapindex = 77,
      .speed = 55,
      .flags = AUTOPILOT_AUTOGUN | AUTOPILOT_GUNZOMBIE | AUTOPILOT_PILZOMBIE |
               AUTOPILOT_ROAM | AUTOPILOT_LSENS | AUTOPILOT_CHASETARG |
               AUTOPILOT_WAS_CHASE_ON | AUTOPILOT_SWARMCHARGE |
               AUTOPILOT_ASSIGNED_TARGET,
      .engaged = true,
  };
  fixture_autopilot = &autopilot;
  fixture_map = 77;

  BtechAutopilotStatus status;
  if (btech_autopilot_status(&context, 0, &status) != BTECH_AUTOPILOT_OK ||
      status.container_unit != -1 || status.associated_unit != -1 ||
      status.map != 77 ||
      status.association != BTECH_AUTOPILOT_ASSOCIATION_NONE ||
      !status.engaged || status.speed_percent != 55 ||
      status.order_count != 0 ||
      status.order_capacity != BTECH_AUTOPILOT_ORDER_CAPACITY ||
      !status.modes.autogun_enabled || !status.modes.autogun_suspended ||
      !status.modes.piloting_suspended || !status.modes.roaming ||
      !status.modes.manual_sensors || !status.modes.chasing_target ||
      !status.modes.chase_resume_pending || !status.modes.swarm_charging ||
      !status.modes.assigned_target)
    return 1;

  autopilot.mapindex = 0;
  fixture_map = -1;
  if (btech_autopilot_status(&context, 0, &status) != BTECH_AUTOPILOT_OK ||
      status.map != -1)
    return 2;
  fixture_map = 0;
  if (btech_autopilot_status(&context, 0, &status) != BTECH_AUTOPILOT_OK ||
      status.map != 0)
    return 3;

  fixture_order_count = 1;
  fixture_validation_result = BTECH_AUTOPILOT_UNAVAILABLE;
  status = (BtechAutopilotStatus){.map = 12345};
  if (btech_autopilot_status(&context, 0, &status) !=
          BTECH_AUTOPILOT_INVALID_ORDER_QUEUE ||
      status.map != 12345)
    return 4;
  fixture_order_count = 0;
  fixture_validation_result = BTECH_AUTOPILOT_OK;

  BtechAutopilotEventStats stats;
  if (btech_autopilot_event_stats(&context, 0, &stats) != BTECH_AUTOPILOT_OK ||
      stats.goto_count != 2 || stats.follow_count != 3 ||
      stats.roam_count != 4 || stats.total != 9 || stats.leave_count != 0 ||
      stats.command_count != 0 || stats.autogun_count != 0 ||
      stats.sensor_count != 0 || stats.enter_base_count != 0 ||
      stats.reply_count != 0 || stats.profile_count != 0)
    return 5;
  if (btech_autopilot_status(&context, 1, &status) !=
          BTECH_AUTOPILOT_UNAVAILABLE ||
      btech_autopilot_status(&context, 0, nullptr) !=
          BTECH_AUTOPILOT_INVALID_ARGUMENT ||
      btech_autopilot_event_stats(&context, 0, nullptr) !=
          BTECH_AUTOPILOT_INVALID_ARGUMENT)
    return 6;
  return 0;
}
