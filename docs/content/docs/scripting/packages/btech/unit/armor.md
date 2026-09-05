---
title: armor
type: docs
toc_hide: false
---

Returns a live unit's armor and internal values.

## Function

### Synopsis

```lua
btech.unit.armor( unit, section )
```

### Arguments

`DbRef|Object unit`
: The live unit.

`BtechSection|nil section`
: An optional typed constant from [`btech.unit.sections`](../sections/) that is
  valid for the unit.

### Returns

`BtechArmorStatus status`
: The requested armor status.

## Notes

When `section` is omitted, the returned armor, internal-structure, and rear-
armor values are totals across all sections, and the record has no `section`
field.
When present, the returned `section` is the same typed constant and round-trips
into section-taking functions.

## See Also

- [`btech`](../../)
- [`btech.unit`](../)
