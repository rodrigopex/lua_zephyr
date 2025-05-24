#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <zephyr/zbus/zbus.h>
#include <lua_zephyr/utils.h>

#include "channels.h"

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_acc_consumed);

ZBUS_CHAN_ADD_OBS(chan_acc_data_consumed, msub_acc_consumed, 3);

#define REF(_a) &_a
#define LUA_ZBUS_CHAN_DECLARE(_chan)                                                               \
	do {                                                                                       \
		const struct zbus_channel **chan =                                                 \
			lua_newuserdata(L, sizeof(const struct zbus_channel *));                   \
		*chan = REF(_chan);                                                                \
		lua_setfield(L, -2, #_chan);                                                       \
	} while (0)

#define LUA_ZBUS_OBS_DECLARE(_obs)                                                                 \
	do {                                                                                       \
		const struct zbus_observer **obs =                                                 \
			lua_newuserdata(L, sizeof(const struct zbus_observer *));                  \
		*obs = REF(_obs);                                                                  \
		lua_setfield(L, -2, #_obs);                                                        \
	} while (0)

static int chan_pub(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 3) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	const struct zbus_channel **chan = lua_touserdata(L, 1);
	struct msg_acc_data *msg = (struct msg_acc_data *)luaL_checkstring(L, 2);
	int timeout_ms = luaL_checkinteger(L, 3);
	int err = zbus_chan_pub(*chan, msg, K_MSEC(timeout_ms));

	lua_pushinteger(L, err);

	return 1;
}

static int sub_wait_msg(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 3) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	const struct zbus_observer **obs = lua_touserdata(L, 1);
	const struct zbus_channel **chan = lua_touserdata(L, 2);
	int timeout_ms = luaL_checkinteger(L, 3);
	const struct zbus_channel *chan_received;

	struct msg_acc_data_consumed msg;

	int err = zbus_sub_wait_msg(*obs, &chan_received, &msg, K_MSEC(timeout_ms));

	if (chan_received == *chan) {
		lua_pushinteger(L, err);
		lua_pushlstring(L, (const char *)&msg, zbus_chan_msg_size(*chan) + 1);

	} else {
		lua_pushinteger(L, -1);
		lua_pushstring(L, "");
	}

	return 2;
}

static const luaL_Reg zbus[] = {
	{"pub", chan_pub},
	{"sub_wait_msg", sub_wait_msg},
	{NULL, NULL} // Sentinel value to mark the end of the array
};

static int luaopen_zbus(lua_State *L)
{
	luaL_newlib(L, zbus);
	LUA_ZBUS_CHAN_DECLARE(chan_acc_data);
	LUA_ZBUS_CHAN_DECLARE(chan_acc_data_consumed);
	LUA_ZBUS_OBS_DECLARE(msub_acc_consumed);
	return 1;
}

int lua_producer_setup(lua_State *L)
{
	LUA_REQUIRE(base);
	LUA_REQUIRE(zephyr);
	LUA_REQUIRE(struct);
	LUA_REQUIRE(zbus);

	return 0;
}
