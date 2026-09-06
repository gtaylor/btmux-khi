---
title: disengage
type: docs
toc_hide: false
---

Stops an engaged autopilot without discarding its association or orders.

## Function

### Synopsis

```lua
btech.autopilot.disengage( autopilot )
```

### Arguments

`DbRef|Object autopilot`
: A registered BattleTech autopilot.

### Returns

None.

### Raises

- `mux.object.invalid` or `mux.object.unavailable` for an invalid autopilot.
- `btech.operation.failed` with reason `already_disengaged`.

## Notes

The associated unit and full queue remain intact. The authoritative engagement
boolean is saved exactly in version 8 snapshots.

## See Also

- [`btech.autopilot`](../)
- [`engage`](../engage/)
- [`clear_orders`](../clear-orders/)
