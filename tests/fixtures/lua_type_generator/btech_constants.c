#include "lua_fixture.h"

/** @par LuaLS ignore btech __tostring -- Internal constant metamethod. */
static int constant_tostring(lua_State *state [[maybe_unused]]) { return 1; }
/** @par LuaLS ignore btech __eq -- Internal constant metamethod. */
static int constant_equal(lua_State *state [[maybe_unused]]) { return 1; }
/** @par LuaLS ignore btech __newindex -- Internal constant metamethod. */
static int constant_immutable(lua_State *state [[maybe_unused]]) { return 1; }
/** @par LuaLS ignore btech __index -- Internal constant metamethod. */
static int constant_namespace_index(lua_State *state [[maybe_unused]]) {
  return 1;
}

void install_constant_metatables(lua_State *state) {
  lua_pushcfunction(state, constant_tostring);
  lua_setfield(state, -2, "__tostring");
  lua_pushcfunction(state, constant_equal);
  lua_setfield(state, -2, "__eq");
  lua_pushcfunction(state, constant_immutable);
  lua_setfield(state, -2, "__newindex");
  lua_pushcfunction(state, constant_namespace_index);
  lua_setfield(state, -2, "__index");
}
