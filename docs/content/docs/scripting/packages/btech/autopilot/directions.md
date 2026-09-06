---
title: btech.autopilot.directions
type: docs
toc_hide: false
---

`btech.autopilot.directions` is an immutable namespace of typed
`BtechAutopilotDirection` constants used by `ENTER_BASE` orders.

## Function

### Synopsis

```lua
btech.autopilot.directions.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechAutopilotDirection constant`
: The typed direction represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, or use as the wrong
typed argument.

## Notes

Constants belong to their namespace and the Lua runtime that created them.

| Constant | Direction |
| --- | --- |
| `NORTH` | North. |
| `EAST` | East. |
| `SOUTH` | South. |
| `WEST` | West. |

Repeated lookups of the same direction compare equal and stringify to their
uppercase key; other values, constant kinds, and runtimes compare unequal.
Unknown lookups and attempts to modify a constant or the namespace raise
`mux.arg.invalid`. Raw strings, constants of another kind, and values
from another Lua runtime are rejected.

```lua
btech.autopilot.add_order(autopilot, {
  name = btech.autopilot.orders.ENTER_BASE,
  direction = btech.autopilot.directions.NORTH,
})
```

The `direction` returned by `btech.autopilot.status` is the same typed constant
and may be supplied to `add_order` unchanged.

## See Also

- [`btech.autopilot`](../)
- [`btech.autopilot.orders`](../orders/)
- [`btech.autopilot.add_order`](../add-order/)
