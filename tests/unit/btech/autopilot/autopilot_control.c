#include <stdint.h>
#include <string.h>

#include "autopilot.h"
#include "autopilot_argument_list_api.h"
#include "autopilot_control_api.h"
#include "autopilot_order_queue_api.h"
#include "btech/autopilot.h"
#include "btech/context.h"
#include "context_internal.h" // IWYU pragma: keep
#include "mech_api_types.h"
#include "mech_events.h"
#include "mech_identity_api.h"
#include "mech_runtime_api.h"
#include "mux/network/mux_event.h"
#include "mux/objects/db.h"
#include "mux/server/platform.h"
#include "mux/support/doubly_linked_list.h"
#include "registry_api.h"
#include "special_object.h"

typedef struct FakeUnit {
  DbRef dbref;
  DbRef map;
  DbRef claimant;
} FakeUnit;

static Autopilot *fixture_first;
static Autopilot *fixture_second;
static FakeUnit unit_one = {.dbref = 10, .map = 20, .claimant = -1};
static FakeUnit unit_two = {.dbref = 11, .map = 21, .claimant = -1};
static int command_event_count;
static int schedules;
static int stops;

bool btech_context_is_auto(BtechContext *context, DbRef key) {
  return context != nullptr &&
         ((fixture_first != nullptr && fixture_first->mynum == key) ||
          (fixture_second != nullptr && fixture_second->mynum == key));
}

void *btech_context_find_object(BtechContext *context, DbRef key) {
  if (!btech_context_is_auto(context, key))
    return nullptr;
  return fixture_first->mynum == key ? fixture_first : fixture_second;
}

GameDatabase *btech_context_database(BtechContext *context) {
  return context->database;
}

Mech *btech_context_get_mech(BtechContext *context [[maybe_unused]], DbRef d) {
  if (d == unit_one.dbref)
    return (Mech *)&unit_one;
  if (d == unit_two.dbref)
    return (Mech *)&unit_two;
  return nullptr;
}

DbRef mech_dbref(const Mech *mech) { return ((const FakeUnit *)mech)->dbref; }

DbRef mech_map_dbref(const Mech *mech) { return ((const FakeUnit *)mech)->map; }

DbRef mech_autopilot_dbref(const Mech *mech) {
  return ((const FakeUnit *)mech)->claimant;
}

void mech_autopilot_dbref_set(Mech *mech, DbRef autopilot) {
  ((FakeUnit *)mech)->claimant = autopilot;
}

int mux_event_count_type_data(MuxEventScheduler *scheduler [[maybe_unused]],
                              int type, const void *data [[maybe_unused]]) {
  return type == EVENT_AUTOCOM ? command_event_count : 0;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void autopilot_event_schedule(Autopilot *autopilot [[maybe_unused]],
                              MechEventType type,
                              MuxEventCallback callback [[maybe_unused]],
                              int delay [[maybe_unused]],
                              intptr_t data [[maybe_unused]]) {
  if (type == EVENT_AUTOCOM) {
    schedules++;
    command_event_count++;
  }
}
// NOLINTEND(bugprone-easily-swappable-parameters)

void auto_com_event(MuxEvent *event [[maybe_unused]]) {}

void auto_set_comtitle(Autopilot *autopilot [[maybe_unused]],
                       Mech *mech [[maybe_unused]]) {}

void auto_stop_pilot(Autopilot *autopilot) {
  stops++;
  autopilot->engaged = false;
  command_event_count = 0;
}

static void speed_arguments(AutopilotArgumentList *arguments,
                            const char *value) {
  autopilot_argument_list_initialize(arguments, AUTOPILOT_MAX_ARGS);
  autopilot_argument_list_set(arguments, 0, strdup("speed"));
  autopilot_argument_list_set(arguments, 1, strdup(value));
}

static int test_association_and_transitions(BtechContext *context,
                                            Autopilot *first,
                                            Autopilot *second) {
  game_database_object(context->database, first->mynum)->location =
      unit_one.dbref;
  first->mymechnum = -1;
  unit_one.claimant = -1;
  AutopilotAssociationStatus association = autopilot_control_association(first);
  if (association.association != BTECH_AUTOPILOT_ASSOCIATION_READY ||
      association.container != unit_one.dbref || association.associated != -1)
    return 1;
  if (autopilot_control_engage(first) != BTECH_AUTOPILOT_OK ||
      !first->engaged || first->mymech != (Mech *)&unit_one ||
      first->mymechnum != unit_one.dbref || first->mapindex != unit_one.map ||
      unit_one.claimant != first->mynum || schedules != 0)
    return 2;
  if (autopilot_control_engage(first) != BTECH_AUTOPILOT_ALREADY_ENGAGED)
    return 3;
  if (autopilot_control_disengage(first) != BTECH_AUTOPILOT_OK ||
      first->engaged || stops != 1 || first->mymechnum != unit_one.dbref ||
      unit_one.claimant != first->mynum ||
      autopilot_control_disengage(first) != BTECH_AUTOPILOT_ALREADY_DISENGAGED)
    return 4;

  game_database_object(context->database, second->mynum)->location =
      unit_one.dbref;
  second->mymechnum = -1;
  if (autopilot_control_engage(second) != BTECH_AUTOPILOT_CONFLICT ||
      second->engaged || unit_one.claimant != first->mynum)
    return 5;

  game_database_object(context->database, second->mynum)->location =
      unit_two.dbref;
  second->mymechnum = unit_one.dbref;
  unit_one.claimant = -1;
  unit_two.claimant = -1;
  if (autopilot_control_association(second).association !=
          BTECH_AUTOPILOT_ASSOCIATION_READY ||
      autopilot_control_engage(second) != BTECH_AUTOPILOT_OK ||
      second->mymechnum != unit_two.dbref || unit_two.claimant != second->mynum)
    return 6;
  (void)autopilot_control_disengage(second);

  second->mymechnum = unit_one.dbref;
  unit_one.claimant = second->mynum;
  if (autopilot_control_association(second).association !=
          BTECH_AUTOPILOT_ASSOCIATION_CONFLICT ||
      autopilot_control_engage(second) != BTECH_AUTOPILOT_CONFLICT)
    return 7;
  return 0;
}

static int test_queue_guards(Autopilot *autopilot) {
  const AutopilotCommandDefinition SPEED = {
      .name = "speed", .argcount = 1, .command_enum = COMMAND_SPEED};
  AutopilotArgumentList arguments;
  autopilot->engaged = true;
  schedules = 0;
  command_event_count = 0;

  speed_arguments(&arguments, "50");
  if (autopilot_control_enqueue(autopilot, &SPEED, &arguments) !=
      BTECH_AUTOPILOT_OK)
    return 1;
  autopilot_argument_list_destroy(&arguments);
  if (schedules != 1 || command_event_count != 1 ||
      autopilot_order_count(autopilot) != 1)
    return 2;

  speed_arguments(&arguments, "60");
  if (autopilot_control_enqueue(autopilot, &SPEED, &arguments) !=
      BTECH_AUTOPILOT_OK)
    return 3;
  autopilot_argument_list_destroy(&arguments);
  if (schedules != 1 || autopilot_order_count(autopilot) != 2)
    return 4;
  if (autopilot_control_remove(autopilot, 1) != BTECH_AUTOPILOT_ACTIVE_ORDER ||
      autopilot_control_clear(autopilot) != BTECH_AUTOPILOT_ACTIVE_ORDER ||
      autopilot_order_count(autopilot) != 2)
    return 5;
  if (autopilot_control_remove(autopilot, 2) != BTECH_AUTOPILOT_OK ||
      autopilot_order_count(autopilot) != 1 ||
      autopilot_control_remove(autopilot, 0) !=
          BTECH_AUTOPILOT_ORDER_NOT_FOUND ||
      autopilot_order_count(autopilot) != 1)
    return 6;

  autopilot->engaged = false;
  if (autopilot_control_clear(autopilot) != BTECH_AUTOPILOT_OK ||
      autopilot_order_count(autopilot) != 0)
    return 7;
  return 0;
}

static int test_reengage_preserves_queue(BtechContext *context,
                                         Autopilot *autopilot) {
  const AutopilotCommandDefinition SPEED = {
      .name = "speed", .argcount = 1, .command_enum = COMMAND_SPEED};
  AutopilotArgumentList arguments;

  game_database_object(context->database, autopilot->mynum)->location =
      unit_one.dbref;
  autopilot->engaged = false;
  autopilot->mymech = (Mech *)&unit_one;
  autopilot->mymechnum = unit_one.dbref;
  autopilot->mapindex = -1;
  unit_one.claimant = -1;
  schedules = 0;
  command_event_count = 0;
  autopilot_order_clear(autopilot);
  if (autopilot_control_association(autopilot).association !=
      BTECH_AUTOPILOT_ASSOCIATION_READY)
    return 1;

  speed_arguments(&arguments, "50");
  if (autopilot_control_enqueue(autopilot, &SPEED, &arguments) !=
      BTECH_AUTOPILOT_OK) {
    autopilot_argument_list_destroy(&arguments);
    return 2;
  }
  autopilot_argument_list_destroy(&arguments);
  if (autopilot_order_count(autopilot) != 1 || schedules != 0 ||
      command_event_count != 0)
    return 3;

  if (autopilot_control_engage(autopilot) != BTECH_AUTOPILOT_OK ||
      !autopilot->engaged || autopilot->mymech != (Mech *)&unit_one ||
      autopilot->mymechnum != unit_one.dbref ||
      autopilot->mapindex != unit_one.map ||
      unit_one.claimant != autopilot->mynum ||
      autopilot_order_count(autopilot) != 1 || schedules != 1 ||
      command_event_count != 1)
    return 4;

  if (autopilot_control_engage(autopilot) != BTECH_AUTOPILOT_ALREADY_ENGAGED ||
      autopilot_order_count(autopilot) != 1 || schedules != 1 ||
      command_event_count != 1)
    return 5;

  autopilot->engaged = false;
  autopilot_order_clear(autopilot);
  return 0;
}

int main(void) {
  GameObject objects[16] = {0};
  GameDatabase database = {.object_storage = objects, .top = 15, .size = 15};
  MuxEventScheduler events = {.first_by_type = nullptr};
  BtechContext context = {.database = &database, .events = &events};
  Autopilot first = {
      .xcode = {.type = GTYPE_AUTO, .context = &context},
      .mynum = 1,
      .commands = doubly_linked_list_create_list(),
  };
  Autopilot second = {
      .xcode = {.type = GTYPE_AUTO, .context = &context},
      .mynum = 2,
      .commands = doubly_linked_list_create_list(),
  };
  fixture_first = &first;
  fixture_second = &second;

  const int ASSOCIATION =
      test_association_and_transitions(&context, &first, &second);
  const int REENGAGE =
      ASSOCIATION == 0 ? test_reengage_preserves_queue(&context, &first) : 0;
  const int QUEUE =
      ASSOCIATION == 0 && REENGAGE == 0 ? test_queue_guards(&first) : 0;
  autopilot_order_clear(&first);
  autopilot_order_clear(&second);
  doubly_linked_list_destroy_list(first.commands);
  doubly_linked_list_destroy_list(second.commands);
  if (ASSOCIATION != 0)
    return 10 + ASSOCIATION;
  if (REENGAGE != 0)
    return 30 + REENGAGE;
  return QUEUE == 0 ? 0 : 40 + QUEUE;
}
