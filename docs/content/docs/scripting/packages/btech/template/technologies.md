---
title: technologies
type: docs
toc_hide: false
---

Lists a template's configured and inferred technologies.

## Function

### Synopsis

```lua
btech.template.technologies( reference )
```

### Arguments

`string reference`
: The unit-template reference.

### Returns

`BtechTechnology[] technologies`
: The technology records. `code` is a typed constant from
  [`btech.unit.technology`](../../unit/technology/), `group` is one of the
  literal strings `primary`, `secondary`, or `infantry`,
  and `name` remains a human-readable string. Codes can be supplied directly
  to live-unit technology mutators. The descriptive `group` is not a
  `btech.unit.technology_groups` clear-operation constant.

## See Also

- [`btech`](../../)
- [`btech.template`](../)
