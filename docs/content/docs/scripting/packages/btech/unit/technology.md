---
title: btech.unit.technology
type: docs
toc_hide: false
---

`btech.unit.technology` is an immutable namespace of typed
`BtechTechnologyCode` constants.

## Function

### Synopsis

```lua
btech.unit.technology.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechTechnologyCode constant`
: The typed technology code represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, raw strings,
wrong-kind constants, or constants from another runtime.

## Notes

Constants are immutable and local to the runtime that created them.

| Group | Constants |
| --- | --- |
| Primary unit technology | `TRIPLE_STRENGTH_MYOMER`, `CLAN_ANTI_MISSILE`, `INNER_SPHERE_ANTI_MISSILE`, `DOUBLE_HEAT_SINKS`, `MASC`, `CLAN`, `FLIPPABLE_ARMS`, `C3_MASTER`, `C3_SLAVE`, `ARTEMIS_IV`, `ECM`, `BEAGLE_PROBE`, `SALVAGE`, `CARGO`, `SEARCH_LIGHT`, `LIGHT_ACTIVE_PROBE`, `ANTI_AIRCRAFT`, `NO_SENSORS`, `SIXTH_SENSE`, `FERRO_FIBROUS`, `ENDO_STEEL`, `XL_ENGINE`, `ICE_ENGINE`, `SINGLE_HEAT_SINKS`, `LIGHT_ENGINE`, `XXL_ENGINE`, `COMPACT_ENGINE`, `REINFORCED_INTERNAL`, `COMPOSITE_INTERNAL`, `HARDENED_ARMOR`, `CRITICAL_PROOF` |
| Secondary unit technology | `STEALTH_ARMOR`, `HEAVY_FERRO_FIBROUS`, `LASER_REFLECTIVE_ARMOR`, `REACTIVE_ARMOR`, `NULL_SIGNATURE_SYSTEM`, `C3I`, `SUPERCHARGER`, `IMPROVED_JUMP_JETS`, `MECHANICAL_JUMP_JETS`, `COMPACT_HEAT_SINKS`, `LASER_HEAT_SINKS`, `BLOODHOUND_PROBE`, `ANGEL_ECM`, `WATCHDOG`, `LIGHT_FERRO_FIBROUS`, `TAG`, `OMNIMECH`, `ARTEMIS_V`, `CAMOUFLAGE`, `CARRIER`, `WATERPROOF`, `XL_GYRO`, `HEAVY_DUTY_GYRO`, `COMPACT_GYRO`, `TARGETING_COMPUTER`, `SMALL_COCKPIT` |
| Infantry technology | `SWARM_ATTACK`, `MOUNT_FRIENDS`, `ANTI_LEG_ATTACK`, `PURIFIER_STEALTH`, `KAGE_STEALTH`, `ACHILEUS_STEALTH`, `INFILTRATOR_STEALTH`, `INFILTRATOR_II_STEALTH`, `MUST_JETTISON_PACK`, `CAN_JETTISON_PACK` |

Use these values with `add_technology` and `remove_technology`. Infantry
constants require a battlesuit. Repeated lookups of the same technology compare
equal and stringify to their uppercase key; other values, constant kinds, and
runtimes compare unequal. Unknown lookups, mutation, raw strings,
wrong-kind constants, and cross-runtime constants raise `mux.arg.invalid`
where an argument is required.

```lua
for _, configured in ipairs(btech.unit.technologies(unit)) do
  btech.unit.remove_technology(unit, configured.code)
end

btech.unit.add_technology(unit, btech.unit.technology.DOUBLE_HEAT_SINKS)
```

The `code` field returned by unit and template `technologies` queries is the
same typed constant and round-trips without conversion. The human-readable
`name` and descriptive `group` fields remain strings. `group` is `primary`,
`secondary`, or `infantry`; it is distinct from the constants used by
`clear_technologies`.

## See Also

- [`btech.unit`](../)
- [`btech.unit.technologies`](../technologies/)
- [`btech.unit.technology_groups`](../technology-groups/)
