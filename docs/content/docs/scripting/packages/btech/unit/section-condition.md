---
title: section_condition
type: docs
toc_hide: false
---

Returns a live unit section's current condition.

## Function

### Synopsis

```lua
btech.unit.section_condition( unit, section )
```

### Arguments

`DbRef|Object unit`
: The live unit.

`BtechSection section`
: A typed constant from [`btech.unit.sections`](../sections/) that is valid for
  the unit.

### Returns

`"operational"|"destroyed"|"flooded" condition`
: `"operational"`, `"destroyed"`, or `"flooded"`.

## See Also

- [`btech`](../../)
- [`btech.unit`](../)
