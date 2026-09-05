---
title: technologies
type: docs
toc_hide: false
---

Lists a live unit's configured and inferred technologies.

## Function

### Synopsis

```lua
btech.unit.technologies( unit )
```

### Arguments

`DbRef|Object unit`
: The live unit.

### Returns

`BtechTechnology[] technologies`
: The technology records. `code` is a typed constant from
  [`btech.unit.technology`](../technology/), `group` is one of the literal
  strings `primary`, `secondary`, or `infantry`, and `name` remains
  a human-readable string. The code round-trips into `add_technology` and
  `remove_technology`; `group` describes the technology catalog and is not a
  `btech.unit.technology_groups` clear-operation constant.

## See Also

- [`btech`](../../)
- [`btech.unit`](../)
