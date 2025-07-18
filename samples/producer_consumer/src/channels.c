#include "channels.h"

#include <zephyr/zbus/zbus.h>
#include <zephyr/sys/util.h>
#include <lua_zephyr/codec.h>

static const struct lua_object_descriptor acc_data[] = {
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_acc_data, x, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_acc_data, y, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_acc_data, z, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_acc_data, source, LUA_CODEC_VALUE_TYPE_STRING),
};
USER_DATA_WRAPPER_DESC(acc_data);

static const struct lua_object_descriptor consumed[] = {
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_acc_data_consumed, count,
				   LUA_CODEC_VALUE_TYPE_INTEGER),
};
USER_DATA_WRAPPER_DESC(consumed);

static const struct lua_object_descriptor version[] = {
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_version, major, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_version, minor, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_version, patch, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_OBJECT_DESCRIPTOR_PRIM(struct msg_version, hardware_id, LUA_CODEC_VALUE_TYPE_STRING),
};
USER_DATA_WRAPPER_DESC(version);

ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL, (void *)&ud_acc_data,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0, .source = ""));

ZBUS_CHAN_DEFINE(chan_acc_data_consumed, struct msg_acc_data_consumed, NULL, (void *)&ud_consumed,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.count = 0));

ZBUS_CHAN_DEFINE(chan_version, struct msg_version, NULL, (void *)&ud_version, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.major = 3, .minor = 2, .patch = 165, .hardware_id = "QEMU_X86"));
