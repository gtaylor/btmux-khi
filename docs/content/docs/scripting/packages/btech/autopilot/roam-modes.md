---
title: btech.autopilot.roam_modes
type: docs
toc_hide: false
---

`btech.autopilot.roam_modes` is an immutable namespace of typed
`BtechAutopilotRoamMode` constants used by `ROAM` orders.

## Function

### Synopsis

```lua
btech.autopilot.roam_modes.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechAutopilotRoamMode constant`
: The typed roaming mode represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, or use as the wrong
typed argument.

## Notes

Constants belong to their namespace and the Lua runtime that created them.

| Constant | Behavior |
| --- | --- |
| `MAP` | Roam across the current map. |
| `RADIUS` | Roam within a radius of the supplied coordinates. |

Repeated lookups of the same mode compare equal and stringify to their
uppercase key; other values, constant kinds, and runtimes compare unequal.
Unknown lookups and attempts to modify a constant or the namespace raise
`mux.arg.invalid`. Raw strings, constants of another kind, and values
from another Lua runtime are rejected.

```lua
btech.autopilot.add_order(autopilot, {
  name = btech.autopilot.orders.ROAM,
  mode = btech.autopilot.roam_modes.RADIUS,
  x = 12,
  y = 7,
  radius = 5,
})
```

The `mode` returned by `btech.autopilot.status` is the same typed constant and
may be supplied to `add_order` unchanged.

## See Also

- [`btech.autopilot`](../)
- [`btech.autopilot.orders`](../orders/)
- [`btech.autopilot.add_order`](../add-order/)
