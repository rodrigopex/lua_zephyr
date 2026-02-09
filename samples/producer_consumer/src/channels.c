#include "channels.h"
#include <lua_zephyr/lua_msg_descr_pb.h>

/* clang-format off */

LUA_PB_DESCR(MsgAccData);
ZBUS_CHAN_DEFINE(chan_acc_data, MsgAccData, NULL,
		LUA_PB_ZBUS_MSG_DESCR(MsgAccData),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0));

LUA_PB_DESCR(MsgAccDataConsumed);
ZBUS_CHAN_DEFINE(chan_acc_data_consumed, MsgAccDataConsumed, NULL,
		LUA_PB_ZBUS_MSG_DESCR(MsgAccDataConsumed),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.count = 0));

LUA_PB_DESCR(MsgVersion);
ZBUS_CHAN_DEFINE(chan_version, MsgVersion, NULL,
		LUA_PB_ZBUS_MSG_DESCR(MsgVersion),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.major = 4, .minor = 7, .patch = 98, .hardware_id = "RPA9"));

#define LUA_PB_SUBMSG_offset MsgAccData_lua_fields
LUA_PB_DESCR(MsgSensorConfig);
#undef LUA_PB_SUBMSG_offset
ZBUS_CHAN_DEFINE(chan_sensor_config, MsgSensorConfig, NULL,
		LUA_PB_ZBUS_MSG_DESCR(MsgSensorConfig),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.sensor_id = 0, .offset = {0}));

/* clang-format on */
