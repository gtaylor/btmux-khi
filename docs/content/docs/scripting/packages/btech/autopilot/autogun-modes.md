---
title: btech.autopilot.autogun_modes
type: docs
toc_hide: false
---

`btech.autopilot.autogun_modes` is an immutable namespace of typed
`BtechAutopilotAutogunMode` constants used by `AUTO_GUN` orders.

## Function

### Synopsis

```lua
btech.autopilot.autogun_modes.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechAutopilotAutogunMode constant`
: The typed automatic-gunnery mode represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, or use as the wrong
typed argument.

## Notes

Constants belong to their namespace and the Lua runtime that created them.

| Constant | Behavior |
| --- | --- |
| `AUTOMATIC` | Select targets automatically. |
| `OFF` | Disable automatic gunnery. |
| `TARGET` | Attack the supplied target unit. |

Repeated lookups of the same mode compare equal and stringify to their
uppercase key; other values, constant kinds, and runtimes compare unequal.
Unknown lookups and attempts to modify a constant or the namespace raise
`mux.arg.invalid`. Raw strings, constants of another kind, and
values from another Lua runtime are rejected.

```lua
btech.autopilot.add_order(autopilot, {
  name = btech.autopilot.orders.AUTO_GUN,
  mode = btech.autopilot.autogun_modes.TARGET,
  target = enemy,
})
```

The `mode` returned by `btech.autopilot.status` is the same typed constant and
may be supplied to `add_order` unchanged.

## See Also

- [`btech.autopilot`](../)
- [`btech.autopilot.orders`](../orders/)
- [`btech.autopilot.add_order`](../add-order/)
