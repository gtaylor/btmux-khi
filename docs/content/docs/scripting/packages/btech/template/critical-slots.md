---
title: critical_slots
type: docs
toc_hide: false
---

Lists a template section's critical slots.

## Function

### Synopsis

```lua
btech.template.critical_slots( reference, section )
```

### Arguments

`string reference`
: The unit-template reference.

`BtechSection section`
: A typed constant from [`btech.unit.sections`](../../unit/sections/) that is
  valid for the template.

### Returns

`BtechCriticalSlot[] slots`
: The critical-slot records. Each record's `section`, `fire_modes`, and
  `ammunition_modes` fields contain typed constants from the corresponding
  `btech.unit` namespaces and can be reused as live-unit inputs.

## See Also

- [`btech`](../../)
- [`btech.template`](../)
