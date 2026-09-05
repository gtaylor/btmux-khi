---
title: btech.autopilot package
linkTitle: autopilot
type: docs
weight: 5
---

`btech.autopilot` provides typed, notification-free administration of an
autopilot's association, engagement, order queue, runtime modes, and events.

Orders are strict tables discriminated by a typed `name` constant; unknown
fields and raw-string discriminators are rejected.
Coordinates must be integers from 0 through 2147483647. Unit targets must
currently be registered BattleTech units, although mutable execution conditions
such as map bounds, terrain, damage, startup state, and carrier relationships
are checked only when the order executes.

| Function | Description |
| --- | --- |
| [`status`](status/) | Return association, queue, and runtime modes. |
| [`add_order`](add-order/) | Append a validated typed order. |
| [`remove_order`](remove-order/) | Remove an order by one-based position. |
| [`clear_orders`](clear-orders/) | Clear a safely inactive queue. |
| [`engage`](engage/) | Associate and start an installed autopilot. |
| [`disengage`](disengage/) | Stop activity while preserving association and orders. |
| [`event_stats`](event-stats/) | Count pending autopilot events. |

## Constants

| Namespace | Description |
| --- | --- |
| [`orders`](orders/) | Order-name discriminators. |
| [`directions`](directions/) | Base-entry directions. |
| [`roam_modes`](roam-modes/) | Roaming behaviors. |
| [`autogun_modes`](autogun-modes/) | Automatic-gunnery behaviors. |

Mutators return no values. Failures raise structured errors; operation failures
use `btech.operation.failed` with a stable `error.detail.reason`.
Every function requires the documented arguments and ignores surplus positional
arguments, consistently with the rest of the native BTech API.
