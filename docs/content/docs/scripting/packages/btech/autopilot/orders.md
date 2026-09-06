---
title: btech.autopilot.orders
type: docs
toc_hide: false
---

`btech.autopilot.orders` is an immutable namespace of typed
`BtechAutopilotOrderName` constants. Use these values as the `name`
discriminator accepted by [`add_order`](../add-order/); raw strings are
rejected.

## Function

### Synopsis

```lua
btech.autopilot.orders.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechAutopilotOrderName constant`
: The typed order name represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, or use as the wrong
typed argument.

## Notes

Constants belong to their namespace and the Lua runtime that created them.

| Constant | Order |
| --- | --- |
| `CHASE_TARGET` | Chase a target unit. |
| `DUMB_FOLLOW` | Follow a target without pathfinding. |
| `FOLLOW` | Follow a target using normal navigation. |
| `EMBARK` | Embark into a target unit. |
| `PICK_UP` | Pick up a target unit. |
| `DUMB_GOTO` | Move directly to map coordinates. |
| `GOTO` | Navigate to map coordinates. |
| `OLD_GOTO` | Use the legacy coordinate-navigation behavior. |
| `ENTER_BASE` | Enter a base from a direction. |
| `LEAVE_BASE` | Leave a base on a heading. |
| `ROAM` | Roam a map or radius. |
| `AUTO_GUN` | Configure automatic gunnery. |
| `DROP_OFF` | Drop off a carried unit. |
| `SHUT_DOWN` | Shut down the associated unit. |
| `START_UP` | Start the associated unit. |
| `UNIT_DISEMBARK` | Disembark the associated unit. |
| `SPEED` | Set the desired speed percentage. |

Repeated lookups of the same order compare equal and stringify to their
uppercase key; other values, constant kinds, and runtimes compare unequal.
Unknown lookups and attempts to modify a constant or the namespace raise
`mux.arg.invalid`. Constants belong to their namespace and current Lua
runtime; a raw string or a different kind of BTech constant is rejected.

```lua
btech.autopilot.add_order(autopilot, {
  name = btech.autopilot.orders.GOTO,
  x = 12,
  y = 7,
})
```

Status snapshots return these same typed constants in each order's `name`
field, so values round-trip directly back into `add_order`.

## See Also

- [`btech.autopilot`](../)
- [`btech.autopilot.add_order`](../add-order/)
- [`btech.autopilot.status`](../status/)
