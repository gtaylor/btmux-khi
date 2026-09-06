---
title: status
type: docs
toc_hide: false
---

Returns a fresh structured snapshot of an autopilot.

## Function

### Synopsis

```lua
local status = btech.autopilot.status( autopilot )
```

### Arguments

`DbRef|Object autopilot`
: A registered BattleTech autopilot.

### Returns

`BtechAutopilotStatus`
: A new table containing optional `Object` fields `container_unit`,
  `associated_unit`, and `map`; `association` (`none`, `ready`, `associated`, or
  `conflict`); `engaged`; `speed_percent`; `order_count`; `order_capacity`
  (always 100); a dense typed `orders` array; and `modes`.

The modes are `autogun_enabled`, `autogun_suspended`, `piloting_suspended`,
`roaming`, `manual_sensors`, `chasing_target`, `chase_resume_pending`,
`swarm_charging`, and `assigned_target`, all booleans.

### Raises

- `mux.object.invalid` if the object is not a registered autopilot or a stored
  object relationship is corrupt.
- `mux.object.unavailable` if its runtime state is unavailable.
- `btech.operation.failed` with reason `invalid_order_queue` if stored orders
  cannot be decoded safely.

## Notes

Each call returns new tables. Object relationships are native `Object` handles,
not numeric database references; absent relationships are `nil`.
Order `name`, `direction`, and `mode` fields are typed constants from the
corresponding `btech.autopilot` namespaces. They compare and stringify like the
input constants and can be passed back to `add_order` without conversion.

## See Also

- [`btech.autopilot`](../)
- [`event_stats`](../event-stats/)
