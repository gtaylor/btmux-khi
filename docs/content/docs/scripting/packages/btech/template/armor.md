---
title: armor
type: docs
toc_hide: false
---

Returns a template's armor and internal values.

## Function

### Synopsis

```lua
btech.template.armor( reference, section )
```

### Arguments

`string reference`
: The unit-template reference.

`BtechSection|nil section`
: An optional typed constant from [`btech.unit.sections`](../../unit/sections/)
  that is valid for the template.

### Returns

`BtechArmorStatus status`
: The requested armor status.

## Notes

When `section` is omitted, the returned armor, internal-structure, and rear-
armor values are totals across all sections, and the record has no `section`
field.
When present, the returned `section` is the same typed constant and can be used
with live-unit section functions.

## See Also

- [`btech`](../../)
- [`btech.template`](../)
