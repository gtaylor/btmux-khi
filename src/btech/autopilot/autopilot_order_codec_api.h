/* Internal conversion between typed orders and legacy queue records. */

#pragma once

#include "autopilot.h"
#include "btech/autopilot.h"

BtechAutopilotResult
autopilot_order_encode(BtechContext *context, const BtechAutopilotOrder *order,
                       const AutopilotCommandDefinition **definition,
                       AutopilotArgumentList *arguments);
BtechAutopilotResult autopilot_order_decode(const AutopilotCommand *command,
                                            BtechAutopilotOrder *order);
