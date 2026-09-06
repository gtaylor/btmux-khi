---
title: btech.unit.sections
type: docs
toc_hide: false
---

`btech.unit.sections` is an immutable namespace of typed `BtechSection`
constants. Section-taking functions validate that the selected section exists
for the target unit or template.

## Function

### Synopsis

```lua
btech.unit.sections.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechSection constant`
: The typed section represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, or use as the wrong
typed argument. Section-taking functions also reject sections absent from the
selected layout.

## Notes

Constants are immutable and local to the runtime that created them.

| Layout | Constants |
| --- | --- |
| Mech | `HEAD`, `LEFT_ARM`, `RIGHT_ARM`, `LEFT_TORSO`, `RIGHT_TORSO`, `CENTER_TORSO`, `LEFT_LEG`, `RIGHT_LEG` |
| Quad | `HEAD`, `FRONT_LEFT_LEG`, `FRONT_RIGHT_LEG`, `LEFT_TORSO`, `RIGHT_TORSO`, `CENTER_TORSO`, `REAR_LEFT_LEG`, `REAR_RIGHT_LEG` |
| Battlesuit | `SUIT_1` through `SUIT_8` |
| Vehicle, naval, or VTOL | `LEFT_SIDE`, `RIGHT_SIDE`, `FRONT_SIDE`, `AFT_SIDE`, `TURRET`, `ROTOR` as supported by the unit |
| Aerospace | `NOSE`, `LEFT_WING`, `RIGHT_WING`, `AFT_SIDE` |
| Aerodyne DropShip | `NOSE`, `LEFT_WING`, `RIGHT_WING`, `LEFT_REAR_WING`, `RIGHT_REAR_WING`, `AFT` |
| Spheroid DropShip | `NOSE`, `FRONT_LEFT_SIDE`, `FRONT_RIGHT_SIDE`, `REAR_LEFT_SIDE`, `REAR_RIGHT_SIDE`, `AFT` |

Repeated lookups of the same section compare equal and stringify to their
uppercase key; other values, constant kinds, and runtimes compare unequal.
Unknown lookups and mutation raise `mux.arg.invalid`. Raw strings, wrong-kind
constants, cross-runtime constants, and sections that do not exist on the
selected unit also raise `mux.arg.invalid` when passed to a function.

```lua
local armor = btech.unit.armor(unit, btech.unit.sections.CENTER_TORSO)
btech.unit.set_armor(unit, armor.section, { armor = 30 })
```

Section fields returned by unit, template, and repair queries use these same
typed constants and round-trip directly into section-taking functions.

## See Also

- [`btech.unit`](../)
- [`btech.unit.armor`](../armor/)
- [`btech.unit.critical_slots`](../critical-slots/)
