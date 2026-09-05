---
title: Unit administration
type: docs
toc_hide: false
---

These functions configure live units using typed values and request tables.
All section, unit-type, movement-type, technology, and mode names are matched
without regard to ASCII letter case.

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
  section = "Right Torso",
  slots = { 1, 2 },
  rear_facing = false,
  targeting_computer = false,
  one_shot = false,
})
```

`part` is a `BtechPartRef`. Slots are one-based, unique, and must provide the
weapon's required critical space. Large weapons may provide a partial section
when the remaining criticals are installed separately.

## install_special

```lua
btech.unit.install_special(unit, {
  part = equipment,
  section = "Center Torso",
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

Restores the default critical-slot layout for every unit section.

## configure_ammunition

```lua
btech.unit.configure_ammunition(unit, {
  weapon = weapon,
  section = "Left Torso",
  slot = 4,
  half_ton = false,
  ammunition_modes = { "Artemis/Mine" },
})
```

Configures and fills an ammunition bin for the referenced weapon. The weapon
must use ammunition. Mode names are the names returned by `critical_slots`.

## restock_ammunition

```lua
btech.unit.restock_ammunition(unit, section, slot)
```

Fills an operational ammunition bin to its calculated capacity.

## set_weapon_modes

```lua
btech.unit.set_weapon_modes(unit, weapon_number, {
  fire_modes = { "RearMount" },
  ammunition_modes = { "Precision" },
})
```

Replaces the selected mounted weapon's fire and ammunition mode sets. Omitted
mode arrays are empty sets.

## add_technology

```lua
btech.unit.add_technology(unit, technology)
```

Adds a technology by the `code` returned from `technologies`. Infantry
technologies require a battlesuit.

## remove_technology

```lua
btech.unit.remove_technology(unit, technology)
```

Removes a technology by the `code` returned from `technologies`. Infantry
technologies require a battlesuit.

## clear_technologies

```lua
btech.unit.clear_technologies(unit, group)
```

`group` is `unit`, `infantry`, or `all`. Clearing unit technologies also
removes equipment and section configuration directly associated with them.

## set_unit_type

```lua
btech.unit.set_unit_type(unit, unit_type)
```

Unit types use the canonical template names. Setting a unit type also selects
its standard movement type where one is intrinsic to that type.

## set_movement_type

```lua
btech.unit.set_movement_type(unit, movement_type)
```

Movement types use the canonical template names.

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
