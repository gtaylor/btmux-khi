---
title: btech.repair
linkTitle: btech.repair
type: docs
weight: 20
no_list: true
sidebar_root_for: self
---

`btech.repair` provides repair state and technician availability.

[`btech.repair.operations`](operations/) provides the immutable typed constants
used by repair records and immediate repair requests.

## Functions

| Function | Description |
| --- | --- |
| [`apply`](apply/) | Performs an immediate administrative repair. |
| [`is_fixable`](is-fixable/) | Tests whether a live unit can be repaired. |
| [`is_under_repair`](is-under-repair/) | Tests whether a live unit has an active repair event. |
| [`needs`](needs/) | Lists the repairs needed by a live unit. |
| [`technician_available_in`](technician-available-in/) | Returns the time until a player's technician is available. |
