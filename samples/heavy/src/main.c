#include <zephyr/kernel.h>

#include <lauxlib.h>
#include <lualib.h>
#include <lua_zephyr/utils.h>

int heavy_lua_setup(lua_State *L)
{
	LUA_REQUIRE(string);
	LUA_REQUIRE(zephyr);
	return 0;
}
