---
title: remove_order
type: docs
toc_hide: false
---

Removes one queued autopilot order.

## Function

### Synopsis

```lua
btech.autopilot.remove_order( autopilot, position )
```

### Arguments

`DbRef|Object autopilot`
: A registered BattleTech autopilot.

`integer position`
: The one-based queue position, from 1 through 100.

### Returns

None.

### Raises

- `mux.arg.invalid` if `position` is not an integer in range.
- `mux.object.invalid` or `mux.object.unavailable` for an invalid autopilot.
- `btech.operation.failed` with reason `active_order` when position 1 is
  protected or `order_not_found` when no such position exists.

## Notes

Position 1 is protected whenever an engaged autopilot has queued orders. Later
pending orders can still be removed.

## See Also

- [`btech.autopilot`](../)
- [`add_order`](../add-order/)
- [`clear_orders`](../clear-orders/)
