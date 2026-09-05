---
title: needs
type: docs
toc_hide: false
---

Lists the repairs needed by a live unit.

## Function

### Synopsis

```lua
btech.repair.needs( unit )
```

### Arguments

`DbRef|Object unit`
: The live unit.

### Returns

`BtechRepairNeed[] needs`
: The required and in-progress repair operations. Each `operation` is a typed
  [`BtechRepairOperation`](../operations/) constant and each `section` is a
  typed [`BtechSection`](../../unit/sections/) constant; both round-trip into
  `btech.repair.apply` for supported immediate repairs.

## Notes

Scheduled repair discovery preserves compatibility with stored unit data. It
may report rear-armor jobs for any section whose configured current and original
rear values differ. Scheduled technician work can restore configured stored
rear armor for ordinary deficits where the current value is below the original
value. Outside a Mech's center, left, and right torsos, stored rear armor is not
consumed by combat and cannot be changed through the immediate
[`btech.repair.apply`](../apply/) operation.

## See Also

- [`btech`](../../)
- [`btech.repair`](../)
- [`btech.repair.apply`](../apply/)
