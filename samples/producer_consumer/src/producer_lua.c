/**
 * @file producer_lua.c
 * @brief Producer-consumer sample: Lua-side producer setup and zbus declarations.
 *
 * Provides the _lua_setup hook for the producer Lua thread.  Loads the
 * zephyr and zbus libraries and exposes the sample channels and observers
 * to Lua.
 */

#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <zephyr/zbus/zbus.h>
#include <luaz_utils.h>
#include <luaz_zbus.h>

#include "channels.h"

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_acc_consumed);

ZBUS_CHAN_ADD_OBS(chan_acc_data_consumed, msub_acc_consumed, 3);

/** @brief Setup hook for the producer Lua thread (loads libs, declares channels). */
int producer_lua_setup(lua_State *L)
{
	LUA_REQUIRE(zephyr);
	LUA_REQUIRE(zbus);

	LUA_ZBUS_CHAN_DECLARE(chan_acc_data);
	LUA_ZBUS_CHAN_DECLARE(chan_acc_data_consumed);
	LUA_ZBUS_CHAN_DECLARE(chan_version);
	LUA_ZBUS_CHAN_DECLARE(chan_sensor_config);
	LUA_ZBUS_OBS_DECLARE(msub_acc_consumed);
	return 0;
}
