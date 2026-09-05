---
title: weapons
type: docs
toc_hide: false
---

Lists the weapons mounted on a live unit.

## Function

### Synopsis

```lua
btech.unit.weapons( unit, section )
```

### Arguments

`DbRef|Object unit`
: The live unit.

`BtechSection|nil section`
: An optional typed constant from [`btech.unit.sections`](../sections/) that is
  valid for the unit.

### Returns

`BtechMountedWeapon[] weapons`
: The mounted weapons. Each `section` field is a typed `BtechSection` constant
  and can be reused as an input.

## See Also

- [`btech`](../../)
- [`btech.unit`](../)
