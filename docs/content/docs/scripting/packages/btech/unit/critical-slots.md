---
title: critical_slots
type: docs
toc_hide: false
---

Lists a live unit section's critical slots.

## Function

### Synopsis

```lua
btech.unit.critical_slots( unit, section )
```

### Arguments

`DbRef|Object unit`
: The live unit.

`BtechSection section`
: A typed constant from [`btech.unit.sections`](../sections/) that is valid for
  the unit.

### Returns

`BtechCriticalSlot[] slots`
: The critical-slot records. Each record's `section`, `fire_modes`, and
  `ammunition_modes` fields contain typed constants from the corresponding
  `btech.unit` namespaces and can be reused as inputs without conversion.

## See Also

- [`btech`](../../)
- [`btech.unit`](../)
