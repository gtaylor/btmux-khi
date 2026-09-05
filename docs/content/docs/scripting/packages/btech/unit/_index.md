---
title: btech.unit
linkTitle: btech.unit
type: docs
weight: -20
no_list: true
sidebar_root_for: self
---

`btech.unit` provides live-unit state, combat queries, and mutations.

## Constants

| Namespace | Description |
| --- | --- |
| [`types`](types/) | Unit classes. |
| [`movement_types`](movement-types/) | Movement classes. |
| [`sections`](sections/) | Unit-layout sections. |
| [`technology`](technology/) | Technology codes. |
| [`technology_groups`](technology-groups/) | Technology-clear groups. |
| [`fire_modes`](fire-modes/) | Weapon fire modes. |
| [`ammunition_modes`](ammunition-modes/) | Ammunition modes. |

## Functions

| Function | Description |
| --- | --- |
| [`apply_damage`](apply-damage/) | Applies clustered damage to a live unit. |
| [`armor`](armor/) | Returns a live unit's armor and internal values. |
| [`assigned_pilot`](assigned-pilot/) | Returns a live unit's assigned pilot. |
| [`battle_value`](battle-value/) | Calculates a live unit's current battle value. |
| [`critical_slots`](critical-slots/) | Lists a live unit section's critical slots. |
| [`display_name`](display-name/) | Returns a live unit's display-name override. |
| [`effective_max_speed`](effective-max-speed/) | Returns a live unit's effective maximum speed. |
| [`effective_max_speed_kph`](administration/#effective_max_speed_kph) | Returns effective maximum speed in kilometers per hour. |
| [`engine`](engine/) | Returns a live unit's engine configuration. |
| [`installed_parts`](installed-parts/) | Lists parts represented by non-destroyed critical slots on a live unit. |
| [`load_template`](load-template/) | Loads a unit template into a live unit. |
| [`restore`](administration/#restore) | Restores a live unit from its model template. |
| [`save_template`](administration/#save_template) | Saves a live unit as a template. |
| [`install_weapon`](administration/#install_weapon) | Installs a weapon into critical slots. |
| [`install_special`](administration/#install_special) | Configures a special-equipment critical slot. |
| [`reset_critical_slots`](administration/#reset_critical_slots) | Restores default critical slots. |
| [`configure_ammunition`](administration/#configure_ammunition) | Configures an ammunition bin. |
| [`restock_ammunition`](administration/#restock_ammunition) | Refills an ammunition bin. |
| [`set_weapon_modes`](administration/#set_weapon_modes) | Replaces a mounted weapon's modes. |
| [`add_technology`](administration/#add_technology) | Adds a configured technology. |
| [`remove_technology`](administration/#remove_technology) | Removes a configured technology. |
| [`clear_technologies`](administration/#clear_technologies) | Clears configured technology groups. |
| [`markings`](markings/) | Returns a live unit's markings. |
| [`payload`](payload/) | Lists weapons and ammunition represented by non-destroyed critical slots on a live unit. |
| [`piloting_check`](piloting-check/) | Makes a piloting check and applies a failed result. |
| [`preferred_id`](preferred-id/) | Returns a live unit's preferred tactical ID. |
| [`radio_channels`](radio-channels/) | Lists a live unit's configured radio channels. |
| [`section_condition`](section-condition/) | Returns a live unit section's current condition. |
| [`set_armor`](set-armor/) | Updates selected armor fields for one live unit section. |
| [`set_assigned_pilot`](set-assigned-pilot/) | Sets or clears a live unit's assigned pilot. |
| [`set_display_name`](set-display-name/) | Sets or clears a live unit's display-name override. |
| [`set_markings`](set-markings/) | Sets or clears a live unit's markings. |
| [`set_max_speed`](set-max-speed/) | Sets a live unit's maximum speed. |
| [`set_jump_speed`](administration/#set_jump_speed) | Sets jump speed in movement points. |
| [`set_unit_type`](administration/#set_unit_type) | Sets the unit type. |
| [`set_movement_type`](administration/#set_movement_type) | Sets the movement type. |
| [`set_heat_sinks`](administration/#set_heat_sinks) | Sets heat sink count. |
| [`set_long_range_sensor_range`](administration/#sensor-ranges) | Sets long-range sensor range. |
| [`set_tactical_range`](administration/#sensor-ranges) | Sets tactical sensor range. |
| [`set_scan_range`](administration/#sensor-ranges) | Sets scan range. |
| [`set_radio_quality`](administration/#radio) | Sets radio quality and derived configuration. |
| [`set_radio_range`](administration/#radio) | Sets radio range. |
| [`set_cargo_capacity`](administration/#set_cargo_capacity) | Sets cargo capacity. |
| [`set_preferred_id`](set-preferred-id/) | Sets or clears a live unit's preferred tactical ID. |
| [`set_tonnage`](set-tonnage/) | Sets a live unit's tonnage and original weight. |
| [`technologies`](technologies/) | Lists a live unit's configured and inferred technologies. |
| [`tic_weapons`](tic-weapons/) | Lists the weapons assigned to a target-interlock circuit. |
| [`weapons`](weapons/) | Lists the weapons mounted on a live unit. |
