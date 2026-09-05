---
title: set_armor
type: docs
toc_hide: false
---

Configures selected armor fields for one live unit section.

## Function

### Synopsis

```lua
btech.unit.set_armor( unit, section, patch )
```

### Arguments

`DbRef|Object unit`
: The live unit.

`BtechSection section`
: A typed constant from [`btech.unit.sections`](../sections/) that is valid for
  the unit.

`table patch`
: One or more of `armor`, `internal`, and `rear_armor`.

### Returns

None.

## Notes

Every supplied value must be an integer from 0 through 255.
Each supplied value becomes both the section's current and original value.
`rear_armor` remains configurable on any existing section for compatibility
with stored unit data. Outside a Mech's center, left, and right torsos, that
value is inert in combat and the immediate [`btech.repair.apply`](../../repair/apply/)
operation refuses to change it; scheduled technician repairs may still restore
the configured stored value.

## See Also

- [`btech`](../../)
- [`btech.unit`](../)
