---
title: show_critical_status
type: docs
toc_hide: false
---

Sends a template's critical-status report to a player.

## Function

### Synopsis

```lua
btech.template.show_critical_status( reference, player, section )
```

### Arguments

`string reference`
: The unit-template reference.

`DbRef|Object player`
: The report recipient.

`BtechSection section`
: A typed constant from [`btech.unit.sections`](../../unit/sections/) that is
  valid for the template.

### Returns

None.

## See Also

- [`btech`](../../)
- [`btech.template`](../)
