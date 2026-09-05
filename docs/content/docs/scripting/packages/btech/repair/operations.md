---
title: btech.repair.operations
type: docs
toc_hide: false
---

`btech.repair.operations` is an immutable namespace of typed
`BtechRepairOperation` constants.

## Function

### Synopsis

```lua
btech.repair.operations.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechRepairOperation constant`
: The typed operation represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, raw strings,
wrong-kind constants, or constants from another runtime.

## Notes

Constants belong to their namespace and the Lua runtime that created them.

| Constants |
| --- |
| `REATTACH`, `REPAIR_PART`, `REPAIR_WEAPON_TEMPORARY`, `REPAIR_ENHANCEMENT`, `REPAIR_FOCUS`, `REPAIR_CRYSTAL`, `REPAIR_BARREL`, `REPAIR_AMMO_FEED`, `REPAIR_RANGING`, `REPAIR_AMMO_MOUNT`, `REPLACE_WEAPON`, `RELOAD`, `REPAIR_ARMOR`, `REPAIR_REAR_ARMOR`, `REPAIR_INTERNAL`, `DETACH`, `SCRAP_PART`, `SCRAP_WEAPON`, `UNLOAD`, `RESEAL`, `REPLACE_SUIT` |

`btech.repair.apply` accepts the supported immediate-repair subset:
`REATTACH`, `REPAIR_PART`, `REPAIR_ARMOR`, `REPAIR_REAR_ARMOR`, and
`REPAIR_INTERNAL`. Other constants describe jobs returned by
`btech.repair.needs` and are rejected by `apply`.

Repeated lookups of the same operation compare equal and stringify to their
uppercase key; other values, constant kinds, and runtimes compare unequal.
Unknown lookups and attempts to modify a constant or the namespace raise
`mux.arg.invalid`. Raw strings, constants of another kind, and values from
another Lua runtime are rejected.

```lua
local repair = btech.repair.needs(unit)[1]
if repair and repair.operation == btech.repair.operations.REPAIR_ARMOR then
  btech.repair.apply(unit, {
    operation = repair.operation,
    section = repair.section,
    value = repair.amount,
  })
end
```

Operation fields returned by `needs` are these same typed constants and
round-trip directly into `apply` when that operation supports immediate repair.

## See Also

- [`btech.repair`](../)
- [`btech.repair.apply`](../apply/)
- [`btech.repair.needs`](../needs/)
