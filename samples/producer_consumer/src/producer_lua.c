#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <zephyr/zbus/zbus.h>
#include <lua_zephyr/utils.h>
#include <lua_zephyr/zbus.h>
#include <lua_zephyr/codec.h>
#include <string.h>

#include "channels.h"

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_acc_consumed);

ZBUS_CHAN_ADD_OBS(chan_acc_data_consumed, msub_acc_consumed, 3);

int producer_lua_setup(lua_State *L)
{
	LUA_REQUIRE(zephyr);
	LUA_REQUIRE(zbus);

	LUA_ZBUS_CHAN_DECLARE(chan_acc_data);
	LUA_ZBUS_CHAN_DECLARE(chan_acc_data_consumed);
	LUA_ZBUS_CHAN_DECLARE(chan_version);
	LUA_ZBUS_OBS_DECLARE(msub_acc_consumed);
	return 0;
}
