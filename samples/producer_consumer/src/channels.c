#include "channels.h"
#include <lua_zephyr/lua_msg_descr_pb.h>

/* clang-format off */

LUA_PB_DESCR(msg_acc_data, MSG_ACC_DATA, struct msg_acc_data);
ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL,
		LUA_ZBUS_MSG_DESCR(struct msg_acc_data, msg_acc_data_lua_fields),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0));

LUA_PB_DESCR(msg_acc_data_consumed, MSG_ACC_DATA_CONSUMED, struct msg_acc_data_consumed);
ZBUS_CHAN_DEFINE(chan_acc_data_consumed, struct msg_acc_data_consumed, NULL,
		LUA_ZBUS_MSG_DESCR(struct msg_acc_data_consumed, msg_acc_data_consumed_lua_fields),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.count = 0));

LUA_PB_DESCR(msg_version, MSG_VERSION, struct msg_version);
ZBUS_CHAN_DEFINE(chan_version, struct msg_version, NULL,
		LUA_ZBUS_MSG_DESCR(struct msg_version, msg_version_lua_fields),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.major = 4, .minor = 7, .patch = 98, .hardware_id = "RPA9"));

#define LUA_PB_SUBMSG_offset msg_acc_data_lua_fields
LUA_PB_DESCR(msg_sensor_config, MSG_SENSOR_CONFIG, struct msg_sensor_config);
#undef LUA_PB_SUBMSG_offset
ZBUS_CHAN_DEFINE(chan_sensor_config, struct msg_sensor_config, NULL,
		LUA_ZBUS_MSG_DESCR(struct msg_sensor_config, msg_sensor_config_lua_fields),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.sensor_id = 0, .offset = {0}));

/* clang-format on */
