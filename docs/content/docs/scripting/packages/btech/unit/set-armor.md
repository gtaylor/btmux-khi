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

`string section`
: A class-specific section name or abbreviation.

`table patch`
: One or more of `armor`, `internal`, and `rear_armor`.

### Returns

None.

## Notes

Every supplied value must be an integer from 0 through 255.
Each supplied value becomes both the section's current and original value.

## See Also

- [`btech`](../../)
- [`btech.unit`](../)
