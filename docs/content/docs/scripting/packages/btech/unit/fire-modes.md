---
title: btech.unit.fire_modes
type: docs
toc_hide: false
---

`btech.unit.fire_modes` is an immutable namespace of typed `BtechFireMode`
constants.

## Function

### Synopsis

```lua
btech.unit.fire_modes.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechFireMode constant`
: The typed fire mode represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, raw strings,
wrong-kind constants, or constants from another runtime.

## Notes

Constants are immutable and local to the runtime that created them.

| Constants |
| --- |
| `DESTROYED`, `DISABLED`, `BROKEN`, `DAMAGED`, `TARGETING_COMPUTER`, `REAR_MOUNT`, `HOTLOAD`, `HALF_TON`, `ONE_SHOT`, `ONE_SHOT_USED`, `ULTRA`, `RAPID_FIRE`, `GATLING`, `ROTARY_TWO_SHOT`, `ROTARY_FOUR_SHOT`, `ROTARY_SIX_SHOT`, `HEAT`, `BACKPACK`, `JETTISONED`, `OMNI_BASE`, `ROCKET_FIRED` |

Mode arrays accepted by `set_weapon_modes` contain these constants. Repeated
lookups of the same mode compare equal and stringify to their uppercase key;
other values, constant kinds, and runtimes compare unequal. Unknown lookups,
mutation, raw strings, wrong-kind constants, and cross-runtime constants raise
`mux.arg.invalid` where an argument is required.

```lua
btech.unit.set_weapon_modes(unit, 0, {
  fire_modes = { btech.unit.fire_modes.REAR_MOUNT },
})
```

Fire-mode arrays returned by critical-slot queries contain the same constants
and round-trip without conversion.

## See Also

- [`btech.unit`](../)
- [`btech.unit.critical_slots`](../critical-slots/)
- [`btech.unit administration`](../administration/)
