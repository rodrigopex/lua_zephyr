#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <zephyr/zbus/zbus.h>
#include <lua_zephyr/utils.h>

ZBUS_CHAN_DECLARE(chan_random_input);

static int chan_random_input_pub(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 2) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	struct {
		int data;
	} msg = {.data = (int)luaL_checkinteger(L, 1)};

	int err = zbus_chan_pub(&chan_random_input, &msg, K_MSEC(luaL_checkinteger(L, 2)));

	lua_pushinteger(L, err);

	return 1;
}

static const luaL_Reg c_wrappers[] = {
	{"pub_random_input", chan_random_input_pub},
	{NULL, NULL} // Sentinel value to mark the end of the array
};

static int luaopen_zbus(lua_State *L)
{
	luaL_newlib(L, c_wrappers);
	return 1;
}

int lua_producer_setup(lua_State *L)
{
	LUA_REQUIRE(zephyr);
	LUA_REQUIRE(zbus);

	return 0;
}
