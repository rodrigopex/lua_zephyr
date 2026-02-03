#include "channels.h"

#include <zephyr/zbus/zbus.h>
#include <zephyr/sys/util.h>
#include <lua_zephyr/codec.h>

/* An example of a table to struct mapping descriptor. This maps a Lua table
 * with fields 'x', 'y', 'z' and 'source' to the struct msg_acc_data. The process of encoding
 * and decoding is then automated using this descriptor. */
static const struct lua_zephyr_table_descr acc_data[] = {
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_acc_data, x, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_acc_data, y, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_acc_data, z, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_acc_data, source, LUA_CODEC_VALUE_TYPE_STRING),
};
LUA_ZEPHYR_WRAPPER_DESC(acc_data); /* This creates a user_data_wrapper that's embedded
				    * in the channel definition below */

static const struct lua_zephyr_table_descr acc_data_array[] = {
	LUA_TABLE_FIELD_DESCRIPTOR_ARRAY(struct msg_acc_data_array, data,
					 LUA_CODEC_VALUE_TYPE_INTEGER, count),
};
LUA_ZEPHYR_WRAPPER_DESC(acc_data_array);

static const struct lua_zephyr_table_descr consumed[] = {
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_acc_data_consumed, count,
					LUA_CODEC_VALUE_TYPE_INTEGER),
};
LUA_ZEPHYR_WRAPPER_DESC(consumed);

static const struct lua_zephyr_table_descr version[] = {
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_version, major, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_version, minor, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_version, patch, LUA_CODEC_VALUE_TYPE_INTEGER),
	LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_version, hardware_id,
					LUA_CODEC_VALUE_TYPE_STRING),
};
LUA_ZEPHYR_WRAPPER_DESC(version);

/* The actual channel definitions, embedding the user_data_wrapper
 * created above. */
ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL, (void *)&ud_acc_data,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.x = 0, .y = 0, .z = 0, .source = ""));

ZBUS_CHAN_DEFINE(chan_acc_data_array, struct msg_acc_data_array, NULL, (void *)&ud_acc_data_array,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.count = 0, .data = {0, 0, 0}));

ZBUS_CHAN_DEFINE(chan_acc_data_consumed, struct msg_acc_data_consumed, NULL, (void *)&ud_consumed,
		 ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.count = 0));

ZBUS_CHAN_DEFINE(chan_version, struct msg_version, NULL, (void *)&ud_version, ZBUS_OBSERVERS_EMPTY,
		 ZBUS_MSG_INIT(.major = 3, .minor = 2, .patch = 165, .hardware_id = "QEMU_X86"));
