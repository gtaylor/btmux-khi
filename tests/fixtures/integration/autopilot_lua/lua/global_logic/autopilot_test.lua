local function expect_error(code, reason, fn, ...)
  local ok, err = mux.error.pcall(fn, ...)
  assert(not ok and err.code == code)
  if reason then
    assert(type(err.detail) == "table" and err.detail.reason == reason)
  end
end

local function exact_keys(value, names)
  local expected = {}
  for _, name in ipairs(names) do
    expected[name] = true
  end
  for name in pairs(value) do
    assert(expected[name], "unexpected field " .. tostring(name))
    expected[name] = nil
  end
  assert(next(expected) == nil)
end

local function test_orders(autopilot, unit)
  local order_names = btech.autopilot.orders
  local directions = btech.autopilot.directions
  local roam_modes = btech.autopilot.roam_modes
  local autogun_modes = btech.autopilot.autogun_modes
  local orders = {
    { name = order_names.CHASE_TARGET, target = unit },
    { name = order_names.DUMB_FOLLOW, target = unit:dbref() },
    { name = order_names.DUMB_GOTO, x = 0, y = 2147483647 },
    { name = order_names.ENTER_BASE, direction = directions.WEST },
    { name = order_names.FOLLOW, target = unit },
    { name = order_names.GOTO, x = 12, y = 34 },
    { name = order_names.LEAVE_BASE, heading = 359 },
    { name = order_names.OLD_GOTO, x = 56, y = 78 },
    { name = order_names.ROAM, mode = roam_modes.MAP },
    { name = order_names.ROAM, mode = roam_modes.RADIUS, x = 9, y = 10, radius = 30 },
    { name = order_names.AUTO_GUN, mode = autogun_modes.AUTOMATIC },
    { name = order_names.AUTO_GUN, mode = autogun_modes.OFF },
    { name = order_names.AUTO_GUN, mode = autogun_modes.TARGET, target = unit },
    { name = order_names.DROP_OFF },
    { name = order_names.EMBARK, target = unit },
    { name = order_names.PICK_UP, target = unit },
    { name = order_names.SHUT_DOWN },
    { name = order_names.SPEED, percent = 100 },
    { name = order_names.START_UP },
    { name = order_names.UNIT_DISEMBARK },
  }
  for _, order in ipairs(orders) do
    assert(select("#", btech.autopilot.add_order(autopilot, order, "ignored")) == 0)
  end
  local status = btech.autopilot.status(autopilot, "ignored")
  assert(status.order_count == #orders and #status.orders == #orders)
  for index, expected in ipairs(orders) do
    local actual = status.orders[index]
    assert(actual.name == expected.name)
    for key, value in pairs(expected) do
      if key ~= "target" then
        assert(actual[key] == value)
      end
    end
    if expected.target then
      assert(actual.target:dbref() == unit:dbref())
    end
  end
  assert(select("#", btech.autopilot.clear_orders(autopilot, "ignored")) == 0)
  for _, order in ipairs(status.orders) do
    assert(select("#", btech.autopilot.add_order(autopilot, order)) == 0)
  end
  assert(btech.autopilot.status(autopilot).order_count == #orders)
  assert(select("#", btech.autopilot.clear_orders(autopilot)) == 0)
end

local function test_constants()
  local order_names = btech.autopilot.orders
  local function assert_protected(value)
    local marker = getmetatable(value)
    assert(type(marker) == "string")
    assert(not pcall(setmetatable, {}, marker))
    assert(not pcall(function()
      ---@diagnostic disable-next-line: inject-field
      marker.__index = function()
        return nil
      end
    end))
  end
  local function assert_catalog(namespace, names)
    assert(type(namespace) == "userdata")
    assert_protected(namespace)
    for _, name in ipairs(names) do
      assert(type(namespace[name]) == "userdata" and tostring(namespace[name]) == name)
      assert_protected(namespace[name])
    end
  end
  assert_catalog(order_names, {
    "CHASE_TARGET",
    "DUMB_FOLLOW",
    "FOLLOW",
    "EMBARK",
    "PICK_UP",
    "DUMB_GOTO",
    "GOTO",
    "OLD_GOTO",
    "ENTER_BASE",
    "LEAVE_BASE",
    "ROAM",
    "AUTO_GUN",
    "DROP_OFF",
    "SHUT_DOWN",
    "START_UP",
    "UNIT_DISEMBARK",
    "SPEED",
  })
  assert_catalog(btech.autopilot.directions, { "NORTH", "EAST", "SOUTH", "WEST" })
  assert_catalog(btech.autopilot.roam_modes, { "MAP", "RADIUS" })
  assert_catalog(btech.autopilot.autogun_modes, { "AUTOMATIC", "OFF", "TARGET" })
  assert(order_names.GOTO == order_names.GOTO)
  assert(not (order_names.GOTO == btech.autopilot.directions.NORTH))
  expect_error("mux.arg.invalid", nil, function()
    ---@diagnostic disable-next-line: undefined-field
    return order_names.NOT_AN_ORDER
  end)
  expect_error("mux.arg.invalid", nil, function()
    return order_names[1]
  end)
  expect_error("mux.arg.invalid", nil, function()
    order_names.GOTO = order_names.SPEED
  end)
  expect_error("mux.arg.invalid", nil, function()
    ---@diagnostic disable-next-line: inject-field
    order_names.GOTO.value = 1
  end)
  assert(order_names.GOTO == order_names.GOTO and tostring(order_names.GOTO) == "GOTO")

  local public_access = mux.world.access.PUBLIC
  local traverse_lock = mux.world.locks.TRAVERSE
  assert_protected(mux.world.access)
  assert_protected(public_access)
  assert_protected(mux.world.locks)
  assert_protected(traverse_lock)
  expect_error("mux.access.invalid", nil, function()
    mux.world.access.PUBLIC = mux.world.access.GOD
  end)
  expect_error("mux.access.invalid", nil, function()
    ---@diagnostic disable-next-line: inject-field
    public_access.value = 1
  end)
  expect_error("mux.arg.invalid", nil, function()
    mux.world.locks.TRAVERSE = mux.world.locks.USE
  end)
  expect_error("mux.arg.invalid", nil, function()
    ---@diagnostic disable-next-line: inject-field
    traverse_lock.value = 1
  end)
  expect_error("mux.arg.invalid", nil, function()
    ---@diagnostic disable-next-line: undefined-field
    return mux.world.locks.NOT_A_LOCK
  end)
  assert(public_access == mux.world.access.PUBLIC and tostring(public_access) == "PUBLIC")
  assert(traverse_lock == mux.world.locks.TRAVERSE and tostring(traverse_lock) == "TRAVERSE")
end

local function autopilot_test(ctx, autopilot_one_dbref, autopilot_two_dbref, unit_one_dbref, unit_two_dbref)
  local order_names = btech.autopilot.orders
  local directions = btech.autopilot.directions
  local roam_modes = btech.autopilot.roam_modes
  local autogun_modes = btech.autopilot.autogun_modes
  local autopilot_one = mux.world.object(assert(tonumber(autopilot_one_dbref)))
  local autopilot_two = mux.world.object(assert(tonumber(autopilot_two_dbref)))
  local unit_one = mux.world.object(assert(tonumber(unit_one_dbref)))
  local unit_two = mux.world.object(assert(tonumber(unit_two_dbref)))
  mux.world.pemit(ctx.enactor, "Autopilot Lua started")
  test_constants()

  expect_error("btech.operation.failed", "not_installed_in_unit", btech.autopilot.engage, autopilot_one)
  mux.world.teleport_object({ object = autopilot_one, destination = unit_one })

  local status = btech.autopilot.status(autopilot_one)
  exact_keys(status, {
    "container_unit",
    "association",
    "engaged",
    "speed_percent",
    "order_count",
    "order_capacity",
    "orders",
    "modes",
  })
  exact_keys(status.modes, {
    "autogun_enabled",
    "autogun_suspended",
    "piloting_suspended",
    "roaming",
    "manual_sensors",
    "chasing_target",
    "chase_resume_pending",
    "swarm_charging",
    "assigned_target",
  })
  assert(status.container_unit:dbref() == unit_one:dbref())
  assert(status.associated_unit == nil and status.map == nil)
  assert(status.association == "ready" and not status.engaged)
  assert(status.speed_percent >= 1 and status.order_count == 0 and status.order_capacity == 100)
  for _, value in pairs(status.modes) do
    assert(value == false)
  end
  local first_orders = status.orders
  rawset(first_orders, "extra", true)
  assert(rawget(btech.autopilot.status(autopilot_one).orders, "extra") == nil)

  local stats = btech.autopilot.event_stats(autopilot_one)
  exact_keys(stats, {
    "goto",
    "leave",
    "command",
    "autogun",
    "sensor",
    "follow",
    "enter_base",
    "reply",
    "profile",
    "roam",
    "total",
  })
  for _, value in pairs(stats) do
    assert(value == 0)
  end
  mux.world.pemit(ctx.enactor, "Autopilot Lua status passed")

  test_orders(autopilot_one, unit_two)
  mux.world.pemit(ctx.enactor, "Autopilot Lua orders passed")
  local invalid = {
    { name = order_names.SPEED, percent = 0 },
    { name = order_names.SPEED, percent = 1.5 },
    { name = order_names.GOTO, x = -1, y = 0 },
    { name = order_names.GOTO, x = 0, y = 0, unknown = true },
    { name = order_names.ENTER_BASE, direction = "up" },
    { name = order_names.ENTER_BASE, direction = roam_modes.MAP },
    { name = order_names.LEAVE_BASE, heading = 360 },
    { name = order_names.ROAM, mode = roam_modes.RADIUS, x = 0, y = 0, radius = 31 },
    { name = order_names.ROAM, mode = "anywhere" },
    { name = order_names.ROAM, mode = autogun_modes.OFF },
    { name = order_names.AUTO_GUN, mode = autogun_modes.TARGET },
    { name = order_names.AUTO_GUN, mode = "sometimes" },
    { name = order_names.AUTO_GUN, mode = roam_modes.MAP },
    { name = "speed", percent = 50 },
    { name = "jump", heading = 10 },
    { name = "goto\0junk", x = 0, y = 0 },
    { name = order_names.ENTER_BASE, direction = "east\0" },
    { name = order_names.ROAM, mode = "map\0" },
    { name = directions.NORTH, x = 0, y = 0 },
  }
  local nul_key_order = { name = order_names.SPEED, percent = 50 }
  nul_key_order["name\0junk"] = true
  table.insert(invalid, nul_key_order)
  for _, order in ipairs(invalid) do
    expect_error("mux.arg.invalid", nil, btech.autopilot.add_order, autopilot_one, order)
    assert(btech.autopilot.status(autopilot_one).order_count == 0)
  end
  expect_error("mux.object.invalid", nil, btech.autopilot.add_order, autopilot_one, {
    name = order_names.FOLLOW,
    target = autopilot_two,
  })
  local invalid_dbrefs = {
    tostring(autopilot_one:dbref()),
    autopilot_one:dbref() + 0.5,
    0 / 0,
    1 / 0,
    1e300,
  }
  for _, invalid_dbref in ipairs(invalid_dbrefs) do
    expect_error("mux.object.invalid", nil, btech.autopilot.status, invalid_dbref)
    expect_error("mux.object.invalid", nil, btech.autopilot.add_order, autopilot_one, {
      name = order_names.FOLLOW,
      target = invalid_dbref,
    })
    assert(btech.autopilot.status(autopilot_one).order_count == 0)
  end
  expect_error("mux.arg.invalid", nil, btech.autopilot.add_order, autopilot_one)
  expect_error("mux.arg.invalid", nil, btech.autopilot.status)
  expect_error("mux.object.invalid", nil, btech.autopilot.status, unit_one)
  expect_error("btech.operation.failed", "order_not_found", btech.autopilot.remove_order, autopilot_one, 1)
  expect_error("mux.arg.invalid", nil, btech.autopilot.remove_order, autopilot_one, 0)
  expect_error("mux.arg.invalid", nil, btech.autopilot.remove_order, autopilot_one, 1.5)
  mux.world.pemit(ctx.enactor, "Autopilot Lua errors passed")

  assert(select("#", btech.autopilot.engage(autopilot_one)) == 0)
  status = btech.autopilot.status(autopilot_one)
  assert(status.engaged and status.association == "associated")
  assert(status.associated_unit:dbref() == unit_one:dbref())
  expect_error("btech.operation.failed", "already_engaged", btech.autopilot.engage, autopilot_one)
  assert(select("#", btech.autopilot.add_order(autopilot_one, { name = order_names.SPEED, percent = 50 })) == 0)
  assert(btech.autopilot.event_stats(autopilot_one).command == 1)
  assert(select("#", btech.autopilot.add_order(autopilot_one, { name = order_names.START_UP })) == 0)
  assert(btech.autopilot.event_stats(autopilot_one).command == 1)
  expect_error("btech.operation.failed", "active_order", btech.autopilot.remove_order, autopilot_one, 1)
  expect_error("btech.operation.failed", "active_order", btech.autopilot.clear_orders, autopilot_one)
  assert(btech.autopilot.status(autopilot_one).order_count == 2)
  assert(select("#", btech.autopilot.remove_order(autopilot_one, 2)) == 0)
  assert(btech.autopilot.status(autopilot_one).order_count == 1)
  assert(select("#", btech.autopilot.disengage(autopilot_one)) == 0)
  expect_error("btech.operation.failed", "already_disengaged", btech.autopilot.disengage, autopilot_one)
  assert(select("#", btech.autopilot.clear_orders(autopilot_one)) == 0)
  mux.world.pemit(ctx.enactor, "Autopilot Lua engagement passed")

  mux.world.teleport_object({ object = autopilot_two, destination = unit_one })
  expect_error("btech.operation.failed", "association_conflict", btech.autopilot.engage, autopilot_two)
  assert(not btech.autopilot.status(autopilot_two).engaged)
  mux.world.pemit(ctx.enactor, "Autopilot Lua conflict passed")

  for _ = 1, 100 do
    assert(select("#", btech.autopilot.add_order(autopilot_one, { name = order_names.SPEED, percent = 50 })) == 0)
  end
  expect_error(
    "btech.operation.failed",
    "queue_full",
    btech.autopilot.add_order,
    autopilot_one,
    { name = order_names.SPEED, percent = 60 }
  )
  assert(btech.autopilot.status(autopilot_one).order_count == 100)
  assert(select("#", btech.autopilot.clear_orders(autopilot_one)) == 0)

  assert(select(
    "#",
    btech.autopilot.add_order(autopilot_one, {
      name = order_names.FOLLOW,
      target = unit_two,
    })
  ) == 0)
  mux.world.destroy_object(unit_two)
  expect_error("btech.operation.failed", "invalid_order_queue", btech.autopilot.status, autopilot_one)
  assert(select("#", btech.autopilot.clear_orders(autopilot_one)) == 0)
  expect_error("mux.object.unavailable", nil, btech.autopilot.add_order, autopilot_one, {
    name = order_names.FOLLOW,
    target = unit_two,
  })
  assert(btech.autopilot.status(autopilot_one).order_count == 0)

  mux.world.pemit(ctx.enactor, "Autopilot Lua passed")
  return true
end

return {
  commands = {
    {
      pattern = "^autopilottest%s+#?(%d+)%s+#?(%d+)%s+#?(%d+)%s+#?(%d+)$",
      access = mux.world.access.WIZARD,
      handler = autopilot_test,
    },
  },
}
