#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <zephyr/zbus/zbus.h>
#include <lua_zephyr/utils.h>
#include <lua_zephyr/zbus.h>
#include <string.h>

#include "channels.h"

ZBUS_MSG_SUBSCRIBER_DEFINE(msub_acc_consumed);

ZBUS_CHAN_ADD_OBS(chan_acc_data_consumed, msub_acc_consumed, 3);

int msg_struct_to_lua_table(lua_State *L, const struct zbus_channel *chan, void *message)
{
	lua_newtable(L);
	if (chan == &chan_acc_data_consumed) {
		struct msg_acc_data_consumed *msg = message;
		LUA_TABLE_SET("type", string, "msg_acc_data_consumed");
		LUA_TABLE_SET("count", integer, msg->count);
	} else if (chan == &chan_version) {
		struct msg_version *msg = message;
		LUA_TABLE_SET("type", string, "msg_version");
		LUA_TABLE_SET("major", integer, msg->major);
		LUA_TABLE_SET("minor", integer, msg->minor);
		LUA_TABLE_SET("patch", integer, msg->patch);
		LUA_TABLE_SET("hardware_id", string, msg->hardware_id);
	}
	return 1;
}

size_t lua_table_to_msg_struct(lua_State *L, void *message)
{
	size_t ret = 0;
	const char *type = LUA_TABLE_GET("type", string);

	if (strcmp(type, "msg_acc_data") == 0) {
		struct msg_acc_data *acc = message;
		acc->x = LUA_TABLE_GET("x", integer);
		acc->y = LUA_TABLE_GET("y", integer);
		acc->z = LUA_TABLE_GET("z", integer);
		ret = sizeof(struct msg_acc_data);
	}

	return 1;
}

int lua_producer_setup(lua_State *L)
{
	LUA_REQUIRE(zephyr);
	LUA_REQUIRE(zbus);

	LUA_ZBUS_CHAN_DECLARE(chan_acc_data);
	LUA_ZBUS_CHAN_DECLARE(chan_acc_data_consumed);
	LUA_ZBUS_CHAN_DECLARE(chan_version);
	LUA_ZBUS_OBS_DECLARE(msub_acc_consumed);
	return 0;
}
