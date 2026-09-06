---
title: btech.unit.ammunition_modes
type: docs
toc_hide: false
---

`btech.unit.ammunition_modes` is an immutable namespace of typed
`BtechAmmunitionMode` constants.

## Function

### Synopsis

```lua
btech.unit.ammunition_modes.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechAmmunitionMode constant`
: The typed ammunition mode represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, raw strings,
wrong-kind constants, or constants from another runtime.

## Notes

Constants are immutable and local to the runtime that created them.

| Constants |
| --- |
| `LBX_CLUSTER`, `ARTEMIS_MINE`, `NARC_SMOKE`, `CLUSTER`, `MINE`, `SMOKE`, `INFERNO`, `SWARM`, `SWARM_1`, `INARC_EXPLOSIVE`, `INARC_HAYWIRE`, `INARC_ECM`, `INARC_NEMESIS`, `ARMOR_PIERCING`, `FLECHETTE`, `INCENDIARY`, `PRECISION`, `STINGER`, `CASELESS`, `SEMI_GUIDED`, `EXTENDED_RANGE`, `HIGH_EXPLOSIVE`, `MML_LRM` |

Mode arrays accepted by `configure_ammunition` and `set_weapon_modes` contain
these constants. Repeated lookups of the same mode compare equal and stringify
to their uppercase key; other values, constant kinds, and runtimes compare
unequal. Unknown lookups, mutation, raw strings, wrong-kind constants, and
cross-runtime constants raise `mux.arg.invalid` where an argument is required.

```lua
btech.unit.configure_ammunition(unit, {
  weapon = weapon,
  section = btech.unit.sections.LEFT_TORSO,
  slot = 4,
  ammunition_modes = { btech.unit.ammunition_modes.ARTEMIS_MINE },
})
```

Ammunition-mode arrays returned by critical-slot queries contain the same
constants and round-trip without conversion.

## See Also

- [`btech.unit`](../)
- [`btech.unit.critical_slots`](../critical-slots/)
- [`btech.unit administration`](../administration/)
