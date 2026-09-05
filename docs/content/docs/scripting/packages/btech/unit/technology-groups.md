---
title: btech.unit.technology_groups
type: docs
toc_hide: false
---

`btech.unit.technology_groups` is an immutable namespace of typed
`BtechTechnologyGroup` constants.

## Function

### Synopsis

```lua
btech.unit.technology_groups.CONSTANT
```

### Arguments

This namespace takes no arguments.

### Returns

`BtechTechnologyGroup constant`
: The typed clearing group represented by `CONSTANT`.

### Raises

Raises `mux.arg.invalid` for invalid lookup, mutation, raw strings,
wrong-kind constants, or constants from another runtime.

## Notes

Constants are immutable and local to the runtime that created them.

| Constant | Group cleared by `clear_technologies` |
| --- | --- |
| `UNIT` | Primary and secondary unit technologies. |
| `INFANTRY` | Battlesuit infantry technologies. |
| `ALL` | Unit and infantry technologies. |

Repeated lookups of the same group compare equal and stringify to their
uppercase key; other values, constant kinds, and runtimes compare unequal.
Unknown lookups, mutation, raw strings, wrong-kind constants, and cross-runtime
constants raise `mux.arg.invalid` where an argument is required.

```lua
btech.unit.clear_technologies(unit, btech.unit.technology_groups.UNIT)
```

These values select what `clear_technologies` removes. They are distinct from
the descriptive `primary`, `secondary`, and `infantry` strings returned in a
technology record's `group` field.

## See Also

- [`btech.unit`](../)
- [`btech.unit.technologies`](../technologies/)
- [`btech.unit.technology`](../technology/)
