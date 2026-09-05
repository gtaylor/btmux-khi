---
title: clear_orders
type: docs
toc_hide: false
---

Clears an autopilot's order queue atomically when safe.

## Function

### Synopsis

```lua
btech.autopilot.clear_orders( autopilot )
```

### Arguments

`DbRef|Object autopilot`
: A registered BattleTech autopilot.

### Returns

None.

### Raises

- `mux.object.invalid` or `mux.object.unavailable` for an invalid autopilot.
- `btech.operation.failed` with reason `active_order` if the engaged queue is
  nonempty.

## Notes

Failure leaves the entire queue unchanged. Disengage first to preserve safety
when clearing a nonempty active queue.

## See Also

- [`btech.autopilot`](../)
- [`disengage`](../disengage/)
- [`remove_order`](../remove-order/)
