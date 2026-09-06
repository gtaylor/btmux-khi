---
title: btech.unit.movement_types
type: docs
toc_hide: false
---

`btech.unit.movement_types` is an immutable namespace of typed
`BtechMovementType` constants.

## Function

### Synopsis

```lua
btech.unit.movement_types.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechMovementType constant`
: The typed movement class represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, or use as the wrong
typed argument.

## Notes

Constants are immutable and local to the runtime that created them.

| Constants |
| --- |
| `BIPED`, `TRACK`, `WHEEL`, `HOVER`, `VTOL`, `HULL`, `FOIL`, `FLY`, `QUAD`, `SUB`, `NONE` |

Use these values with `btech.unit.set_movement_type`. Repeated lookups of the
same movement type compare equal and stringify to their uppercase key; other
values, constant kinds, and runtimes compare unequal. Unknown lookups, mutation,
raw strings, wrong-kind constants, and constants from another Lua runtime raise
`mux.arg.invalid` where an argument is required.

```lua
btech.unit.set_movement_type(unit, btech.unit.movement_types.BIPED)
```

## See Also

- [`btech.unit`](../)
- [`btech.unit administration`](../administration/)
