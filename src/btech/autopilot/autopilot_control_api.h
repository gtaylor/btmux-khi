/* Internal notification-free autopilot control shared with legacy commands. */

#pragma once

#include "autopilot.h"
#include "btech/autopilot.h"

typedef struct AutopilotAssociationStatus {
  BtechAutopilotAssociation association;
  BtechObjectId container;
  BtechObjectId associated;
} AutopilotAssociationStatus;

BtechAutopilotResult autopilot_control_engage(Autopilot *autopilot);
BtechAutopilotResult autopilot_control_disengage(Autopilot *autopilot);
BtechAutopilotResult
autopilot_control_enqueue(Autopilot *autopilot,
                          const AutopilotCommandDefinition *definition,
                          const AutopilotArgumentList *arguments);
BtechAutopilotResult autopilot_control_remove(Autopilot *autopilot,
                                              size_t position);
BtechAutopilotResult autopilot_control_clear(Autopilot *autopilot);
AutopilotAssociationStatus
autopilot_control_association(const Autopilot *autopilot);
