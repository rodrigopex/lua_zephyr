#include "channels.h"
#include <lua_zephyr/lua_msg_descr.h>

/* clang-format off */
static const struct lua_msg_field_descr acc_data_fields[] = {
	LUA_MSG_FIELD(struct msg_acc_data, x, LUA_MSG_TYPE_INT),
	LUA_MSG_FIELD(struct msg_acc_data, y, LUA_MSG_TYPE_INT),
	LUA_MSG_FIELD(struct msg_acc_data, z, LUA_MSG_TYPE_INT),
};
ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL,
		LUA_ZBUS_MSG_DESCR(struct msg_acc_data, acc_data_fields),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0));

static const struct lua_msg_field_descr acc_data_consumed_fields[] = {
	LUA_MSG_FIELD(struct msg_acc_data_consumed, count, LUA_MSG_TYPE_INT),
};
ZBUS_CHAN_DEFINE(chan_acc_data_consumed, struct msg_acc_data_consumed, NULL,
		LUA_ZBUS_MSG_DESCR(struct msg_acc_data_consumed, acc_data_consumed_fields),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.count = 0));

static const struct lua_msg_field_descr version_fields[] = {
	LUA_MSG_FIELD(struct msg_version, major, LUA_MSG_TYPE_UINT),
	LUA_MSG_FIELD(struct msg_version, minor, LUA_MSG_TYPE_UINT),
	LUA_MSG_FIELD(struct msg_version, patch, LUA_MSG_TYPE_UINT),
	LUA_MSG_FIELD(struct msg_version, hardware_id, LUA_MSG_TYPE_STRING),
};
ZBUS_CHAN_DEFINE(chan_version, struct msg_version, NULL,
		LUA_ZBUS_MSG_DESCR(struct msg_version, version_fields),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.major = 4, .minor = 7, .patch = 98, .hardware_id = "RPA9"));

static const struct lua_msg_field_descr sensor_config_fields[] = {
	LUA_MSG_FIELD(struct msg_sensor_config, sensor_id, LUA_MSG_TYPE_INT),
	LUA_MSG_FIELD_OBJECT(struct msg_sensor_config, offset, acc_data_fields),
};
ZBUS_CHAN_DEFINE(chan_sensor_config, struct msg_sensor_config, NULL,
		LUA_ZBUS_MSG_DESCR(struct msg_sensor_config, sensor_config_fields),
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(.sensor_id = 0, .offset = {0}));
/* clang-format on */
