#include "lua_fixture.h"

static int btech_unexpected_handler(lua_State *state [[maybe_unused]]) {
  return 0;
}

void install_btech_unexpected_handler(lua_State *state) {
  lua_pushcfunction(state, btech_unexpected_handler);
  lua_setfield(state, -2, "unexpected");
}
