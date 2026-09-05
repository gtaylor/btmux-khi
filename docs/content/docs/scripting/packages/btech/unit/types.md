---
title: btech.unit.types
type: docs
toc_hide: false
---

`btech.unit.types` is an immutable namespace of typed `BtechUnitType`
constants.

## Function

### Synopsis

```lua
btech.unit.types.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechUnitType constant`
: The typed unit class represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, or use as the wrong
typed argument.

## Notes

Constants are immutable and local to the runtime that created them.

| Constants |
| --- |
| `MECH`, `VEHICLE`, `VTOL`, `NAVAL`, `SPHEROID_DROPSHIP`, `AERO_FIGHTER`, `MECHWARRIOR`, `AERODYNE_DROPSHIP`, `BATTLESUIT` |

Use these values with `btech.unit.set_unit_type`. Repeated lookups of the same
unit type compare equal and stringify to their uppercase key; other values,
constant kinds, and runtimes compare unequal. Unknown lookups, mutation, raw
strings, wrong-kind constants, and constants from another Lua runtime raise
`mux.arg.invalid` where an argument is required.

```lua
btech.unit.set_unit_type(unit, btech.unit.types.MECH)
```

## See Also

- [`btech.unit`](../)
- [`btech.unit administration`](../administration/)
