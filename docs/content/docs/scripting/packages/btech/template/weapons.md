---
title: weapons
type: docs
toc_hide: false
---

Lists the weapons mounted on a unit template.

## Function

### Synopsis

```lua
btech.template.weapons( reference, section )
```

### Arguments

`string reference`
: The unit-template reference.

`BtechSection|nil section`
: An optional typed constant from [`btech.unit.sections`](../../unit/sections/)
  that is valid for the template.

### Returns

`BtechMountedWeapon[] weapons`
: The mounted weapons. Each `section` field is a typed `BtechSection` constant
  and can be reused with live-unit functions.

## See Also

- [`btech`](../../)
- [`btech.template`](../)
