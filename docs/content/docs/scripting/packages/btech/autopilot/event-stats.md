---
title: event_stats
type: docs
toc_hide: false
---

Counts pending events associated with an autopilot.

## Function

### Synopsis

```lua
local stats = btech.autopilot.event_stats( autopilot )
```

### Arguments

`DbRef|Object autopilot`
: A registered BattleTech autopilot.

### Returns

`BtechAutopilotEventStats`
: A fresh table with integer fields `goto`, `leave`, `command`, `autogun`,
  `sensor`, `follow`, `enter_base`, `reply`, `profile`, `roam`, and `total`.
  Every category is present, including zero counts.

### Raises

- `mux.object.invalid` if the object is not a registered autopilot.
- `mux.object.unavailable` if its runtime state is unavailable.

## Notes

The result is a snapshot; later event scheduling does not mutate it.

## See Also

- [`btech.autopilot`](../)
- [`status`](../status/)
