---
title: apply
type: docs
toc_hide: false
---

Performs an immediate repair on a live unit.

## Function

### Synopsis

```lua
btech.repair.apply( unit, repair )
```

### Arguments

`DbRef|Object unit`
: A registered live BattleTech unit.

`table repair`
: A strict operation record. Unknown fields are rejected. Every record requires
  a typed [`BtechSection`](../../unit/sections/) in `section` and one of these
  exact shapes, using [`btech.repair.operations`](../operations/):

  - `{ operation = operations.REPAIR_ARMOR, section = BtechSection, value = integer }`
  - `{ operation = operations.REPAIR_INTERNAL, section = BtechSection, value = integer }`
  - `{ operation = operations.REPAIR_REAR_ARMOR, section = BtechSection, value = integer }`
  - `{ operation = operations.REPAIR_PART, section = BtechSection, slot = integer }`
  - `{ operation = operations.REATTACH, section = BtechSection }`

  Raw operation and section strings are rejected. Armor and internal values must
  be integers from 0 through 255. `slot` is a one-based integer within the
  selected section's current critical-slot count. `REPAIR_REAR_ARMOR` is valid
  only for a Mech's center, left, or right torso (`CTORSO`/`CT`, `LTORSO`/`LT`,
  or `RTORSO`/`RT`); this immediate operation is rejected for every other
  section and unit type.

### Returns

None.

### Raises

- `mux.object.invalid` if `unit` is not a registered BattleTech unit.
- `mux.object.unavailable` if the unit's runtime state is unavailable.
- `mux.arg.invalid` if `repair` is not a table; has an unknown or missing field;
  specifies an unsupported operation or section; or supplies a fractional,
  out-of-range, or otherwise invalid `value` or `slot`.

## Examples

```lua
local unit = mux.world.object(42)

btech.repair.apply(unit, {
  operation = btech.repair.operations.REPAIR_ARMOR,
  section = btech.unit.sections.CENTER_TORSO,
  value = 32,
})

btech.repair.apply(unit, {
  operation = btech.repair.operations.REPAIR_PART,
  section = btech.unit.sections.LEFT_ARM,
  slot = 3,
})
```

## Notes

Repairs apply immediately and do not enqueue technician work. Surplus positional
arguments are ignored consistently with other native `btech` functions.
Scheduled repair discovery remains compatibility-oriented: it may report and
restore configured stored rear armor on other sections even though combat does
not consume those values.

## See Also

- [`btech.repair`](../)
- [`btech.unit`](../../unit/)
- [`btech.repair.operations`](../operations/)
