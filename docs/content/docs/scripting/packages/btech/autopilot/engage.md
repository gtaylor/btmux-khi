---
title: engage
type: docs
toc_hide: false
---

Associates and engages an autopilot installed in a unit.

## Function

### Synopsis

```lua
btech.autopilot.engage( autopilot )
```

### Arguments

`DbRef|Object autopilot`
: A registered BattleTech autopilot physically contained by a registered unit.

### Returns

None.

### Raises

- `mux.object.invalid` or `mux.object.unavailable` for an invalid autopilot.
- `btech.operation.failed` with reason `not_installed_in_unit`,
  `already_engaged`, or `association_conflict`.

## Notes

Another autopilot cannot claim the same unit, and a valid backlink to another
unit is a conflict. Only unambiguous stale cached associations are repaired.
Engagement is persisted explicitly in version 8 snapshots. Version 6 and 7
snapshots infer it once from the autogun or zombie flags, or from matching
physical containment and backlinks when the queue is nonempty; subsequent saves
use version 8. The guarded offline migration at
`game/data/migrations/btech-persistence-v8.sql` applies the same inference. Stop
the server and back up a populated database before running it.

## See Also

- [`btech.autopilot`](../)
- [`disengage`](../disengage/)
- [`status`](../status/)
