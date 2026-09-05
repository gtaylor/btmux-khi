---
title: Unit administration
type: docs
toc_hide: false
---

These functions configure live units using typed constants and request tables.
Raw strings are rejected for sections, unit and movement types, technologies,
technology groups, and fire or ammunition modes.

## restore

```lua
btech.unit.restore(unit)
```

Loads the unit's model template, cancels its events, and clears line-of-sight
state.

## save_template

```lua
btech.unit.save_template(unit, reference)
```

Saves the unit beneath the configured template directory. `reference` must be
a non-empty resource name without path components.

## effective_max_speed_kph

```lua
local speed = btech.unit.effective_max_speed_kph(unit)
```

Returns effective maximum speed in kilometers per hour.

## install_weapon

```lua
btech.unit.install_weapon(unit, {
  part = weapon,
  section = btech.unit.sections.RIGHT_TORSO,
  slots = { 1, 2 },
  rear_facing = false,
  targeting_computer = false,
  one_shot = false,
})
```

`part` is a `BtechPartRef`. Slots are one-based, unique, and must provide the
weapon's required critical space. Large weapons may provide a partial section
when the remaining criticals are installed separately. Each selected slot has
its prior damage and selection/auxiliary metadata cleared. Equipment-derived
metadata for displaced parts is reconciled against the remaining critical-slot
grid.

## install_special

```lua
btech.unit.install_special(unit, {
  part = equipment,
  section = btech.unit.sections.CENTER_TORSO,
  slot = 1,
  auxiliary_data = 0,
})
```

The part must identify special equipment. Omit `part` to empty the slot.
Equipment-derived technology and section configuration are updated.

## reset_critical_slots

```lua
btech.unit.reset_critical_slots(unit)
```

Restores the default critical-slot layout for every unit section and clears
per-slot damage and selection/auxiliary metadata. Equipment technology flags,
CASE section configuration, and Inner Sphere and Clan anti-missile-system flags
are then re-derived from the restored critical-slot grid.

## configure_ammunition

```lua
btech.unit.configure_ammunition(unit, {
  weapon = weapon,
  section = btech.unit.sections.LEFT_TORSO,
  slot = 4,
  half_ton = false,
  ammunition_modes = { btech.unit.ammunition_modes.ARTEMIS_MINE },
})
```

Configures and fills an ammunition bin for the referenced weapon. The weapon
must use ammunition. Mode names are the names returned by `critical_slots`. The
replaced slot has its prior damage and selection/auxiliary metadata cleared,
and equipment-derived metadata for its displaced part is reconciled against
the remaining critical-slot grid.

## restock_ammunition

```lua
btech.unit.restock_ammunition(unit, btech.unit.sections.LEFT_TORSO, slot)
```

Fills an operational ammunition bin to its calculated capacity.

## set_weapon_modes

```lua
btech.unit.set_weapon_modes(unit, weapon_number, {
  fire_modes = { btech.unit.fire_modes.REAR_MOUNT },
  ammunition_modes = { btech.unit.ammunition_modes.PRECISION },
})
```

Replaces the selected mounted weapon's fire and ammunition mode sets. Omitted
mode arrays are empty sets.

## add_technology

```lua
btech.unit.add_technology(unit, technology)
```

Adds a typed constant from `btech.unit.technology`; the `code` returned from
`technologies` is the same type and round-trips directly. Infantry technologies
require a battlesuit.

## remove_technology

```lua
btech.unit.remove_technology(unit, technology)
```

Removes a typed constant from `btech.unit.technology`; the `code` returned from
`technologies` round-trips directly. Infantry technologies require a
battlesuit.

## clear_technologies

```lua
btech.unit.clear_technologies(unit, btech.unit.technology_groups.UNIT)
```

`group` comes from `btech.unit.technology_groups`. Clearing unit technologies
also removes equipment and section configuration directly associated with
them.

## set_unit_type

```lua
btech.unit.set_unit_type(unit, btech.unit.types.MECH)
```

Unit types come from `btech.unit.types`. Setting a unit type also selects its
standard movement type where one is intrinsic to that type.

## set_movement_type

```lua
btech.unit.set_movement_type(unit, btech.unit.movement_types.BIPED)
```

Movement types come from `btech.unit.movement_types`.

## set_jump_speed

```lua
btech.unit.set_jump_speed(unit, movement_points)
```

Jump speed is expressed in movement points.

## set_heat_sinks

```lua
btech.unit.set_heat_sinks(unit, count)
```

Heat sink count is an integer from 0 through 127.

## Sensor ranges

```lua
btech.unit.set_long_range_sensor_range(unit, range)
btech.unit.set_tactical_range(unit, range)
btech.unit.set_scan_range(unit, range)
```

Each range is an integer from 0 through 127 hexes.

## Radio

```lua
btech.unit.set_radio_quality(unit, quality)
btech.unit.set_radio_range(unit, range)
```

Quality is an integer from 1 through 5 and derives the radio configuration and
default range. An explicit radio range is an integer from 0 through 32767.

## set_cargo_capacity

```lua
btech.unit.set_cargo_capacity(unit, space, maximum_tons)
```

`space` is an integer from 0 through 5000. `maximum_tons` is an integer from 1
through 100.

## See Also

- [`btech.unit`](../)
- [`btech.repair.apply`](../../repair/apply/)
