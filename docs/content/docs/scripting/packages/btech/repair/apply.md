---
title: apply
type: docs
toc_hide: false
---

Performs an immediate repair on a live unit.

## Function

```lua
btech.repair.apply(unit, repair)
```

`repair` contains `operation` and `section`. Armor operations
(`repair_armor`, `repair_internal`, and `repair_rear_armor`) also require a
`value` from 0 through 255. `repair_part` requires a one-based `slot`.
`reattach` requires no additional field.

The function returns no values.

## See Also

- [`btech.repair`](../)
- [`btech.unit`](../../unit/)
