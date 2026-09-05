/* btech_contracts.c - Canonical LuaLS contract for the native BTech package. */

/**
 * @par LuaLS definition btech namespace btech
 * @code{.lua}
 * ---The native BattleTech host API. All functions are unavailable during `@lua/check`.
 * ---@class BtechPackage
 * ---@field autopilot BtechAutopilotPackage
 * ---@field character BtechCharacterPackage
 * ---@field map BtechMapPackage
 * ---@field parts BtechPartsPackage
 * ---@field player BtechPlayerPackage
 * ---@field repair BtechRepairPackage
 * ---@field system BtechSystemPackage
 * ---@field template BtechTemplatePackage
 * ---@field unit BtechUnitPackage
 * ---@field error BtechErrorPackage
 * btech = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.autopilot
 * @code{.lua}
 * ---@class BtechAutopilotPackage
 * ---@field orders BtechAutopilotOrderNamespace
 * ---@field directions BtechAutopilotDirectionNamespace
 * ---@field roam_modes BtechAutopilotRoamModeNamespace
 * ---@field autogun_modes BtechAutopilotAutogunModeNamespace
 * local btech_autopilot = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.character
 * @code{.lua}
 * ---@class BtechCharacterPackage
 * local btech_character = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.map
 * @code{.lua}
 * ---@class BtechMapPackage
 * local btech_map = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.parts
 * @code{.lua}
 * ---@class BtechPartsPackage
 * local btech_parts = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.player
 * @code{.lua}
 * ---@class BtechPlayerPackage
 * local btech_player = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.repair
 * @code{.lua}
 * ---@class BtechRepairPackage
 * ---@field operations BtechRepairOperationNamespace
 * local btech_repair = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.system
 * @code{.lua}
 * ---@class BtechSystemPackage
 * local btech_system = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.template
 * @code{.lua}
 * ---@class BtechTemplatePackage
 * local btech_template = {}
 * @endcode
 * @par LuaLS definition btech namespace btech.unit
 * @code{.lua}
 * ---@class BtechUnitPackage
 * ---@field types BtechUnitTypeNamespace
 * ---@field movement_types BtechMovementTypeNamespace
 * ---@field sections BtechSectionNamespace
 * ---@field technology BtechTechnologyCodeNamespace
 * ---@field technology_groups BtechTechnologyGroupNamespace
 * ---@field fire_modes BtechFireModeNamespace
 * ---@field ammunition_modes BtechAmmunitionModeNamespace
 * local btech_unit = {}
 * @endcode
 * @par LuaLS definition btech binding btech.packages
 * @code{.lua}
 * btech.autopilot = btech_autopilot
 * btech.character = btech_character
 * btech.map = btech_map
 * btech.parts = btech_parts
 * btech.player = btech_player
 * btech.repair = btech_repair
 * btech.system = btech_system
 * btech.template = btech_template
 * btech.unit = btech_unit
 * @endcode
 *
 * @par LuaLS definition btech alias btech.part-ref
 * @code{.lua}
 * ---@alias BtechPartRef BtechPart|integer|string
 * @endcode
 * @par LuaLS definition btech alias btech.terrain
 * @code{.lua}
 * ---@alias BtechTerrain "grassland"|"road"|"light_forest"|"heavy_forest"|"water"|"ice"|"bridge"|"high_water"|"rough"|"mountains"|"fire"|"smoke"|"snow"|"building"|"wall"
 * @endcode
 * @par LuaLS definition btech alias btech.line-of-sight
 * @code{.lua}
 * ---@alias BtechLineOfSight "none"|"blocked"|"clear"
 * @endcode
 * @par LuaLS definition btech type btech.constant-values
 * @code{.lua}
 * ---Typed repair operation from [`btech.repair.operations`](lua://btech.repair.operations).
 * ---@class BtechRepairOperation
 * ---Typed unit class from [`btech.unit.types`](lua://btech.unit.types).
 * ---@class BtechUnitType
 * ---Typed movement class from [`btech.unit.movement_types`](lua://btech.unit.movement_types).
 * ---@class BtechMovementType
 * ---Typed unit-layout section from [`btech.unit.sections`](lua://btech.unit.sections).
 * ---@class BtechSection
 * ---Typed technology code from [`btech.unit.technology`](lua://btech.unit.technology).
 * ---@class BtechTechnologyCode
 * ---Typed technology group from [`btech.unit.technology_groups`](lua://btech.unit.technology_groups).
 * ---@class BtechTechnologyGroup
 * ---Typed weapon fire mode from [`btech.unit.fire_modes`](lua://btech.unit.fire_modes).
 * ---@class BtechFireMode
 * ---Typed ammunition mode from [`btech.unit.ammunition_modes`](lua://btech.unit.ammunition_modes).
 * ---@class BtechAmmunitionMode
 * @endcode
 * @par LuaLS definition btech type btech.map-records
 * @code{.lua}
 * ---@class BtechCargoTransferPoint
 * ---@field x integer
 * ---@field y integer
 * ---@field reveal_hint boolean
 * ---@class BtechMapOffsetEntrance
 * ---@field mode "offset"
 * ---@field offset integer
 * ---@class BtechMapExactEntrance
 * ---@field mode "exact"
 * ---@field x integer
 * ---@field y integer
 * ---@class BtechMapEntrances
 * ---@field north? BtechMapEntrance
 * ---@field east? BtechMapEntrance
 * ---@field south? BtechMapEntrance
 * ---@field west? BtechMapEntrance
 * ---@class BtechMapLink
 * ---@field parent DbRef|Object
 * ---@field x integer
 * ---@field y integer
 * ---@field entrances? BtechMapEntrances
 * @endcode
 * @par LuaLS definition btech alias btech.map-entrance
 * @code{.lua}
 * ---@alias BtechMapEntrance BtechMapOffsetEntrance|BtechMapExactEntrance
 * @endcode
 * @par LuaLS definition btech type btech.shared-records
 * @code{.lua}
 * ---@class BtechCharacterValueDefinition
 * ---@field code integer
 * ---@field name string
 * ---@field kind string
 * ---@field default_experience_threshold integer
 * ---@class BtechCharacterValue
 * ---@field definition BtechCharacterValueDefinition
 * ---@field amount integer
 * ---@field target? integer
 * ---@field experience? integer
 * ---@field experience_to_next_level? integer
 * ---@class BtechWeaponStats
 * ---@field kind string
 * ---@field heat integer
 * ---@field damage integer
 * ---@field minimum_range integer
 * ---@field short_range integer
 * ---@field medium_range integer
 * ---@field long_range integer
 * ---@field critical_slots integer
 * ---@field ammunition_per_ton integer
 * ---@field recycle_time integer
 * ---@field battle_value integer
 * ---@class BtechPart
 * ---@field id integer
 * ---@field brand integer
 * ---@field packed_id integer
 * ---@field short_name string
 * ---@field long_name string
 * ---@field very_long_name string
 * ---@field category string
 * ---@field weight_tons number
 * ---@field cost integer
 * ---@field weapon? BtechWeaponStats
 * ---@class BtechPartStack
 * ---@field part BtechPart
 * ---@field quantity integer
 * ---@class BtechValuePair
 * ---@field current integer
 * ---@field original integer
 * ---@class BtechArmorStatus
 * ---@field section? BtechSection
 * ---@field armor BtechValuePair
 * ---@field internal BtechValuePair
 * ---@field rear_armor BtechValuePair
 * ---@class BtechAmmunitionStatus
 * ---@field rounds integer
 * ---@field capacity integer
 * ---@class BtechCriticalSlot
 * ---@field section BtechSection
 * ---@field slot integer
 * ---@field kind string
 * ---@field part? BtechPart
 * ---@field operational boolean
 * ---@field temporary_failure boolean
 * ---@field auxiliary_data integer
 * ---@field ammunition? BtechAmmunitionStatus
 * ---@field fire_modes BtechFireMode[]
 * ---@field ammunition_modes BtechAmmunitionMode[]
 * ---@class BtechMountedWeapon
 * ---@field number integer
 * ---@field section BtechSection
 * ---@field first_slot integer
 * ---@field part BtechPart
 * ---@field slot_count integer
 * ---@field recycle integer
 * ---@field recycle_time integer
 * ---@field operational boolean
 * ---@class BtechEngine
 * ---@field rating integer
 * ---@field suspension_factor integer
 * ---@class BtechRadioChannel
 * ---@field channel integer
 * ---@field frequency integer
 * ---@field title string
 * ---@field modes string[]
 * ---@class BtechBattleValue
 * ---@field total number
 * ---@field offensive number
 * ---@field defensive number
 * ---@class BtechTechnology
 * ---@field code BtechTechnologyCode
 * ---@field name string
 * ---@field group "primary"|"secondary"|"infantry"
 * ---@field source "configured"|"inferred"
 * ---@class BtechRepairNeed
 * ---@field operation BtechRepairOperation
 * ---@field section BtechSection
 * ---@field slot? integer
 * ---@field amount? integer
 * ---@field in_progress boolean
 * ---@class BtechHex
 * ---@field x integer
 * ---@field y integer
 * ---@class BtechPosition: BtechHex
 * ---@field z? integer
 * ---@class BtechPartCategory
 * ---@field code string
 * ---@field name string
 * ---@class BtechBlastZone
 * ---@field x integer
 * ---@field y integer
 * ---@field radius integer
 * ---@class BtechMapEmitOptions
 * ---@field audience? "all"|"range"|"line_of_sight"
 * ---@field origin? BtechPosition
 * ---@field range? number
 * @endcode
 *
 * @par LuaLS definition btech callable btech.character.catalog
 * @code{.lua}
 * ---@param kind string
 * ---@param character? Object
 * ---@return BtechCharacterValueDefinition[] definitions
 * function btech_character.catalog(kind, character) end
 * @endcode
 * @par LuaLS definition btech callable btech.character.value
 * @code{.lua}
 * ---@param character Object
 * ---@param value string|integer
 * ---@return BtechCharacterValue result
 * function btech_character.value(character, value) end
 * @endcode
 * @par LuaLS definition btech callable btech.character.experience_threshold
 * @code{.lua}
 * ---@param skill string
 * ---@return integer threshold
 * function btech_character.experience_threshold(skill) end
 * @endcode
 * @par LuaLS definition btech callable btech.character.set_value
 * @code{.lua}
 * ---@param character Object
 * ---@param value string
 * ---@param amount integer
 * function btech_character.set_value(character, value, amount) end
 * @endcode
 * @par LuaLS definition btech callable btech.character.set_skill_target
 * @code{.lua}
 * ---@param character Object
 * ---@param skill string
 * ---@param target integer
 * function btech_character.set_skill_target(character, skill, target) end
 * @endcode
 * @par LuaLS definition btech callable btech.character.set_skill_experience
 * @code{.lua}
 * ---@param character Object
 * ---@param skill string
 * ---@param experience integer
 * function btech_character.set_skill_experience(character, skill, experience) end
 * @endcode
 * @par LuaLS definition btech callable btech.character.add_skill_experience
 * @code{.lua}
 * ---@param character Object
 * ---@param skill string
 * ---@param amount integer
 * function btech_character.add_skill_experience(character, skill, amount) end
 * @endcode
 *
 * @par LuaLS definition btech callable btech.map.blast_zones
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@return BtechBlastZone[] zones
 * function btech_map.blast_zones(map) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.elevation
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@param hex BtechHex
 * ---@return integer elevation
 * function btech_map.elevation(map, hex) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.terrain
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@param hex BtechHex
 * ---@return BtechTerrain terrain
 * function btech_map.terrain(map, hex) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.emit
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@param message string
 * ---@param options? BtechMapEmitOptions
 * function btech_map.emit(map, message, options) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.in_blast_zone
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@param hex BtechHex
 * ---@return boolean inside
 * function btech_map.in_blast_zone(map, hex) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.line_of_sight
 * @code{.lua}
 * ---@param observer DbRef|Object
 * ---@param target DbRef|Object|BtechHex
 * ---@return BtechLineOfSight state
 * function btech_map.line_of_sight(observer, target) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.unit_by_id
 * @code{.lua}
 * ---@param origin DbRef|Object
 * ---@param id string
 * ---@return Object|nil unit
 * function btech_map.unit_by_id(origin, id) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.load
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@param name string
 * function btech_map.load(map, name) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.range
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@param from DbRef|Object|BtechPosition
 * ---@param to DbRef|Object|BtechPosition
 * ---@return number range
 * function btech_map.range(map, from, to) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.place_unit
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param map DbRef|Object
 * ---@param position BtechPosition
 * function btech_map.place_unit(unit, map, position) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.units
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@param filter? table
 * ---@return Object[] units
 * function btech_map.units(map, filter) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.update_links
 * @code{.lua}
 * ---@param map DbRef|Object
 * function btech_map.update_links(map) end
 * @endcode
 *
 * @par LuaLS definition btech callable btech.parts.categories
 * @code{.lua}
 * ---@return BtechPartCategory[] categories
 * function btech_parts.categories() end
 * @endcode
 * @par LuaLS definition btech callable btech.parts.list
 * @code{.lua}
 * ---@param category? string
 * ---@return BtechPart[] parts
 * function btech_parts.list(category) end
 * @endcode
 * @par LuaLS definition btech callable btech.parts.search
 * @code{.lua}
 * ---@param query string
 * ---@return BtechPart[] parts
 * function btech_parts.search(query) end
 * @endcode
 * @par LuaLS definition btech callable btech.parts.resolve
 * @code{.lua}
 * ---@param part BtechPartRef
 * ---@return BtechPart|nil part
 * function btech_parts.resolve(part) end
 * @endcode
 * @par LuaLS definition btech callable btech.parts.stores
 * @code{.lua}
 * ---@param target DbRef|Object
 * ---@return BtechPartStack[] stores
 * function btech_parts.stores(target) end
 * @endcode
 * @par LuaLS definition btech callable btech.parts.store_quantity
 * @code{.lua}
 * ---@param target DbRef|Object
 * ---@param part BtechPartRef
 * ---@return integer quantity
 * function btech_parts.store_quantity(target, part) end
 * @endcode
 * @par LuaLS definition btech callable btech.parts.adjust_stores
 * @code{.lua}
 * ---@param target DbRef|Object
 * ---@param part BtechPartRef
 * ---@param delta integer
 * function btech_parts.adjust_stores(target, part, delta) end
 * @endcode
 * @par LuaLS definition btech callable btech.parts.set_cost
 * @code{.lua}
 * ---@param part BtechPartRef
 * ---@param cost integer
 * function btech_parts.set_cost(part, cost) end
 * @endcode
 *
 * @par LuaLS definition btech callable btech.repair.needs
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return BtechRepairNeed[] needs
 * function btech_repair.needs(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.repair.is_under_repair
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return boolean under_repair
 * function btech_repair.is_under_repair(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.repair.is_fixable
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return boolean fixable
 * function btech_repair.is_fixable(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.repair.technician_available_in
 * @code{.lua}
 * ---@param player DbRef|Object
 * ---@return integer seconds
 * function btech_repair.technician_available_in(player) end
 * @endcode
 * @par LuaLS definition btech callable btech.system.event_lag
 * @code{.lua}
 * ---@return integer seconds
 * function btech_system.event_lag() end
 * @endcode
 * @par LuaLS definition btech callable btech.system.units_in_zone
 * @code{.lua}
 * ---@param zone DbRef|Object
 * ---@return Object[] units
 * function btech_system.units_in_zone(zone) end
 * @endcode
 *
 * @par LuaLS definition btech callable btech.template.armor
 * @code{.lua}
 * ---@param reference string
 * ---@param section? BtechSection
 * ---@return BtechArmorStatus status
 * function btech_template.armor(reference, section) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.battle_value
 * @code{.lua}
 * ---@param reference string
 * ---@return BtechBattleValue value
 * function btech_template.battle_value(reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.critical_slots
 * @code{.lua}
 * ---@param reference string
 * ---@param section BtechSection
 * ---@return BtechCriticalSlot[] slots
 * function btech_template.critical_slots(reference, section) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.engine
 * @code{.lua}
 * ---@param reference string
 * ---@return BtechEngine engine
 * function btech_template.engine(reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.weapons
 * @code{.lua}
 * ---@param reference string
 * ---@param section? BtechSection
 * ---@return BtechMountedWeapon[] weapons
 * function btech_template.weapons(reference, section) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.installed_parts
 * @code{.lua}
 * ---@param reference string
 * ---@return BtechPartStack[] parts
 * function btech_template.installed_parts(reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.payload
 * @code{.lua}
 * ---@param reference string
 * ---@return BtechPartStack[] parts
 * function btech_template.payload(reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.technologies
 * @code{.lua}
 * ---@param reference string
 * ---@return BtechTechnology[] technologies
 * function btech_template.technologies(reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.base_cost
 * @code{.lua}
 * ---@param reference string
 * ---@return integer cost
 * function btech_template.base_cost(reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.show_status
 * @code{.lua}
 * ---@param reference string
 * ---@param player DbRef|Object
 * function btech_template.show_status(reference, player) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.show_weapon_specs
 * @code{.lua}
 * ---@param reference string
 * ---@param player DbRef|Object
 * function btech_template.show_weapon_specs(reference, player) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.show_critical_status
 * @code{.lua}
 * ---@param reference string
 * ---@param player DbRef|Object
 * ---@param section BtechSection
 * function btech_template.show_critical_status(reference, player, section) end
 * @endcode
 *
 * @par LuaLS definition btech callable btech.unit.armor
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param section? BtechSection
 * ---@return BtechArmorStatus status
 * function btech_unit.armor(unit, section) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.battle_value
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return BtechBattleValue value
 * function btech_unit.battle_value(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.critical_slots
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param section BtechSection
 * ---@return BtechCriticalSlot[] slots
 * function btech_unit.critical_slots(unit, section) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.engine
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return BtechEngine engine
 * function btech_unit.engine(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.weapons
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param section? BtechSection
 * ---@return BtechMountedWeapon[] weapons
 * function btech_unit.weapons(unit, section) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.installed_parts
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return BtechPartStack[] parts
 * function btech_unit.installed_parts(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.payload
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return BtechPartStack[] parts
 * function btech_unit.payload(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.technologies
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return BtechTechnology[] technologies
 * function btech_unit.technologies(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.display_name
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return string|nil name
 * function btech_unit.display_name(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_display_name
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param name string|nil
 * function btech_unit.set_display_name(unit, name) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.load_template
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param reference string
 * function btech_unit.load_template(unit, reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.piloting_check
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param options table
 * ---@return boolean succeeded
 * function btech_unit.piloting_check(unit, options) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.effective_max_speed
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return number movement_points
 * function btech_unit.effective_max_speed(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.effective_max_speed_kph
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return number kilometers_per_hour
 * function btech_unit.effective_max_speed_kph(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.section_condition
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param section BtechSection
 * ---@return "operational"|"destroyed"|"flooded" condition
 * function btech_unit.section_condition(unit, section) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_armor
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param section BtechSection
 * ---@param patch table
 * function btech_unit.set_armor(unit, section, patch) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.apply_damage
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param request table
 * function btech_unit.apply_damage(unit, request) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.radio_channels
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return BtechRadioChannel[] channels
 * function btech_unit.radio_channels(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_max_speed
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param movement_points number
 * function btech_unit.set_max_speed(unit, movement_points) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_tonnage
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param tons integer
 * function btech_unit.set_tonnage(unit, tons) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.tic_weapons
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param tic integer
 * ---@return BtechMountedWeapon[] weapons
 * function btech_unit.tic_weapons(unit, tic) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.cargo_transfer_point
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@return BtechCargoTransferPoint|nil point
 * function btech_map.cargo_transfer_point(map) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.set_cargo_transfer_point
 * @code{.lua}
 * ---@param map DbRef|Object
 * ---@param point BtechCargoTransferPoint|nil
 * function btech_map.set_cargo_transfer_point(map, point) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.link
 * @code{.lua}
 * ---@param child DbRef|Object
 * ---@return BtechMapLink|nil link
 * function btech_map.link(child) end
 * @endcode
 * @par LuaLS definition btech callable btech.map.set_link
 * @code{.lua}
 * ---@param child DbRef|Object
 * ---@param link BtechMapLink|nil
 * function btech_map.set_link(child, link) end
 * @endcode
 * @par LuaLS definition btech callable btech.player.ui_preferences
 * @code{.lua}
 * ---@param player DbRef|Object
 * ---@return BtechUiPreferencesState preferences
 * function btech_player.ui_preferences(player) end
 * @endcode
 * @par LuaLS definition btech callable btech.player.set_ui_preferences
 * @code{.lua}
 * ---@param player DbRef|Object
 * ---@param preferences BtechUiPreferencesState|nil
 * function btech_player.set_ui_preferences(player, preferences) end
 * @endcode
 * @par LuaLS definition btech callable btech.player.mechwarrior_template
 * @code{.lua}
 * ---@param player DbRef|Object
 * ---@return string|nil reference
 * function btech_player.mechwarrior_template(player) end
 * @endcode
 * @par LuaLS definition btech callable btech.player.set_mechwarrior_template
 * @code{.lua}
 * ---@param player DbRef|Object
 * ---@param reference string|nil
 * function btech_player.set_mechwarrior_template(player, reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.player.loadout
 * @code{.lua}
 * ---@param player DbRef|Object
 * ---@return BtechPersonalCombatLoadout|nil loadout
 * function btech_player.loadout(player) end
 * @endcode
 * @par LuaLS definition btech callable btech.player.set_loadout
 * @code{.lua}
 * ---@param player DbRef|Object
 * ---@param loadout BtechPersonalCombatLoadout|nil
 * function btech_player.set_loadout(player, loadout) end
 * @endcode
 * @par LuaLS definition btech callable btech.template.exists
 * @code{.lua}
 * ---@param reference string
 * ---@return boolean exists
 * function btech_template.exists(reference) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.preferred_id
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return string|nil id
 * function btech_unit.preferred_id(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_preferred_id
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param id string|nil
 * function btech_unit.set_preferred_id(unit, id) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.markings
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return string|nil markings
 * function btech_unit.markings(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_markings
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param markings string|nil
 * function btech_unit.set_markings(unit, markings) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.assigned_pilot
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@return Object|nil pilot
 * function btech_unit.assigned_pilot(unit) end
 * @endcode
 * @par LuaLS definition btech callable btech.unit.set_assigned_pilot
 * @code{.lua}
 * ---@param unit DbRef|Object
 * ---@param pilot DbRef|Object|nil
 * function btech_unit.set_assigned_pilot(unit, pilot) end
 * @endcode
 */
[[maybe_unused]] static constexpr int BTECH_LUA_CONTRACTS = 0;
