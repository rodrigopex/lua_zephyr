#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <zephyr/zbus/zbus.h>
#include <lua_zephyr/utils.h>
#include <lua_zephyr/zbus.h>
#include <zephyr/kernel.h>

/* Metatable name for our userdata */
#define ZBUS_CHAN_METATABLE "zbus.channel.mt"
#define ZBUS_OBS_METATABLE  "zbus.observer.mt"

#define ZBUS_MSG_CAPACITY 512

static const struct zbus_channel **check_zbus_channel(lua_State *L, int idx)
{
	void *ud = luaL_checkudata(L, idx, ZBUS_CHAN_METATABLE);
	luaL_argcheck(L, ud != NULL, idx, "`zbus channel' expected");
	return ud;
}

int __weak msg_struct_to_lua_table(lua_State *L, const struct zbus_channel *chan, void *message)
{
	lua_pushnil(L);
	return 1;
}

size_t __weak lua_table_to_msg_struct(lua_State *L, void *message)
{
	return 0;
}

static int chan_pub(lua_State *L)
{
	int err = -EINVAL;

	int n = lua_gettop(L);
	if (n != 3) {
		return luaL_error(L, "expected 3 arguments, got %d", n);
	}

	const struct zbus_channel **chan = check_zbus_channel(L, 1);

	luaL_checktype(L, 2, LUA_TTABLE);

	int timeout_ms = luaL_checkinteger(L, 3);
	uint8_t msg[ZBUS_MSG_CAPACITY] = {};

	size_t s = lua_table_to_msg_struct(L, msg);

	if (s) {
		err = zbus_chan_pub(*chan, msg, K_MSEC(timeout_ms));
	}

	lua_pushinteger(L, err);

	return 1;
}

static int chan_read(lua_State *L)
{
	int err = -EINVAL;

	int n = lua_gettop(L);
	if (n != 2) {
		return luaL_error(L, "expected 2 arguments, got %d", n);
	}

	const struct zbus_channel **chan = check_zbus_channel(L, 1);
	int timeout_ms = luaL_checkinteger(L, 2);

	uint8_t msg[ZBUS_MSG_CAPACITY] = {};

	err = zbus_chan_read(*chan, msg, K_MSEC(timeout_ms));

	lua_pushinteger(L, err);

	msg_struct_to_lua_table(L, *chan, msg);

	return 2;
}
static int chan_equals(lua_State *L)
{
	const struct zbus_channel **chan1 = check_zbus_channel(L, 1);
	if (chan1 == NULL) {
		return luaL_error(L, "invalid zbus channel");
	}
	const struct zbus_channel **chan2 = check_zbus_channel(L, 2);
	if (chan2 == NULL) {
		return luaL_error(L, "invalid zbus channel");
	}

	lua_pushboolean(L, *chan1 == *chan2);

	return 1;
}

static int chan_tostring(lua_State *L)
{
	const struct zbus_channel **chan = check_zbus_channel(L, 1);

	if (chan == NULL) {
		return luaL_error(L, "invalid zbus channel");
	}

	lua_pushfstring(L, "zbus_channel { ref=%p } ", *chan);
	return 1;
}

static const struct luaL_Reg zbus_chan_metamethods[] = {
	{"pub", chan_pub},
	{"read", chan_read},
	{"__tostring", chan_tostring},
	{"__eq", chan_equals},
	{NULL, NULL} // Sentinel
};

static const struct zbus_observer **check_zbus_observer(lua_State *L, int idx)
{
	void *ud = luaL_checkudata(L, idx, ZBUS_OBS_METATABLE);
	luaL_argcheck(L, ud != NULL, idx, "`zbus observer' expected");
	return ud;
}

static int sub_wait_msg(lua_State *L)
{
	const struct zbus_channel *chan;
	uint8_t msg[ZBUS_MSG_CAPACITY] = {};

	const struct zbus_observer **obs = check_zbus_observer(L, 1);
	int timeout_ms = luaL_checkinteger(L, 2);

	int err = zbus_sub_wait_msg(*obs, &chan, msg, K_MSEC(timeout_ms));

	lua_pushinteger(L, err);

	if (err) {
		lua_pushnil(L);
		lua_pushnil(L);
	} else {

		const struct zbus_channel **lua_chan =
			lua_newuserdata(L, sizeof(const struct zbus_channel *));
		*lua_chan = chan;
		luaL_getmetatable(L, ZBUS_CHAN_METATABLE);
		lua_setmetatable(L, -2);

		msg_struct_to_lua_table(L, chan, msg);
	}

	return 3;
}

static const struct luaL_Reg zbus_obs_metamethods[] = {
	{"wait_msg", sub_wait_msg}, {NULL, NULL} // Sentinel
};

static const luaL_Reg zbus[] = {
	{NULL, NULL} // Sentinel value to mark the end of the array
};

int lua_zbus_chan_declare(lua_State *L, const struct zbus_channel *chan, const char *chan_name)
{
	lua_getglobal(L, "zbus");
	if (!lua_istable(L, -1)) {
		return luaL_error(L, "`zbus' is not initialized");
	}

	const struct zbus_channel **chan_ud =
		lua_newuserdata(L, sizeof(const struct zbus_channel *));
	*chan_ud = chan;
	luaL_getmetatable(L, ZBUS_CHAN_METATABLE);
	lua_setmetatable(L, -2);

	lua_setfield(L, -2, chan_name);

	return 1;
}

int lua_zbus_obs_declare(lua_State *L, const struct zbus_observer *obs, const char *obs_name)
{
	lua_getglobal(L, "zbus");
	if (!lua_istable(L, -1)) {
		return luaL_error(L, "`zbus' is not initialized");
	}

	const struct zbus_observer **obs_ud =
		lua_newuserdata(L, sizeof(const struct zbus_observer *));
	*obs_ud = obs;
	luaL_getmetatable(L, ZBUS_OBS_METATABLE);
	lua_setmetatable(L, -2);

	lua_setfield(L, -2, obs_name);

	return 1;
}

int luaopen_zbus(lua_State *L)
{
	luaL_newmetatable(L, ZBUS_CHAN_METATABLE);
	/* Duplicate the metatable for setting __index */
	lua_pushvalue(L, -1);
	/* Set metatable.__index = metatable */
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, zbus_chan_metamethods, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, ZBUS_OBS_METATABLE);
	/* Duplicate the metatable for setting __index */
	lua_pushvalue(L, -1);
	/* Set metatable.__index = metatable */
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, zbus_obs_metamethods, 0);
	lua_pop(L, 1);

	luaL_newlib(L, zbus);

	return 1;
}
