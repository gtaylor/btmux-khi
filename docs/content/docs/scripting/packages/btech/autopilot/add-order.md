---
title: add_order
type: docs
toc_hide: false
---

Appends one validated typed order to an autopilot queue.

## Function

### Synopsis

```lua
btech.autopilot.add_order( autopilot, order )
```

### Arguments

`DbRef|Object autopilot`
: A registered BattleTech autopilot.

`BtechAutopilotOrder order`
: One of the strict shapes below. No additional fields are accepted.

The `name` field must come from [`btech.autopilot.orders`](../orders/).
Directions and modes use the corresponding typed constant namespaces.

```lua
{ name = orders.CHASE_TARGET|orders.DUMB_FOLLOW|orders.FOLLOW|orders.EMBARK|orders.PICK_UP, target = unit }
{ name = orders.DUMB_GOTO|orders.GOTO|orders.OLD_GOTO, x = 0..2147483647, y = 0..2147483647 }
{ name = orders.ENTER_BASE, direction = directions.NORTH|directions.EAST|directions.SOUTH|directions.WEST }
{ name = orders.LEAVE_BASE, heading = 0..359 }
{ name = orders.ROAM, mode = roam_modes.MAP }
{ name = orders.ROAM, mode = roam_modes.RADIUS, x = 0..2147483647, y = 0..2147483647, radius = 1..30 }
{ name = orders.AUTO_GUN, mode = autogun_modes.AUTOMATIC|autogun_modes.OFF }
{ name = orders.AUTO_GUN, mode = autogun_modes.TARGET, target = unit }
{ name = orders.DROP_OFF|orders.SHUT_DOWN|orders.START_UP|orders.UNIT_DISEMBARK }
{ name = orders.SPEED, percent = 1..100 }
```

Here `orders`, `directions`, `roam_modes`, and `autogun_modes` abbreviate their
namespaces beneath `btech.autopilot`; they are not raw strings.

Coordinates are integers from 0 through 2147483647. Each `unit` is a
`DbRef|Object` that must currently resolve to a registered BattleTech unit.

### Returns

None.

### Raises

- `mux.arg.invalid` for a malformed table, unknown field, raw string,
  wrong-kind constant, invalid current-runtime constant, or
  fractional/out-of-range number.
- `mux.object.invalid` or `mux.object.unavailable` for an invalid autopilot or
  target.
- `btech.operation.failed` with reason `queue_full` or `invalid_order_queue`.
- `mux.internal` if native storage cannot be allocated.

## Notes

Map membership and bounds, destroyed state, terrain/path feasibility, carrier
relationships, and startup state are deferred until execution. Appending to an
engaged idle queue schedules its first command exactly once.

## See Also

- [`btech.autopilot`](../)
- [`remove_order`](../remove-order/)
- [`clear_orders`](../clear-orders/)
- [`orders`](../orders/)
