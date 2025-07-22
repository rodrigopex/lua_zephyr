#ifndef _LUA_ZEPHYR_CODEC_H
#define _LUA_ZEPHYR_CODEC_H

#include <lua.h>
#include <zephyr/sys/util.h>

enum lua_codec_value_type {
	LUA_CODEC_VALUE_TYPE_NIL,
	LUA_CODEC_VALUE_TYPE_BOOLEAN,
	LUA_CODEC_VALUE_TYPE_NUMBER,
	LUA_CODEC_VALUE_TYPE_STRING,
	LUA_CODEC_VALUE_TYPE_INTEGER,
};

struct user_data_wrapper {
	const struct lua_zephyr_table_descr *desc;
	size_t desc_size;
};

struct lua_zephyr_table_descr {
	const char *element_name;
	enum lua_codec_value_type type;
	size_t element_name_len;
	size_t offset;
	size_t size;
};

#define LUA_ZEPHYR_WRAPPER_DESC(desc_)                                                             \
	const struct user_data_wrapper ud_##desc_ = {                                              \
		.desc = desc_,                                                                     \
		.desc_size = ARRAY_SIZE(desc_),                                                    \
	}

#define LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct_, field_name_, type_)                               \
	{                                                                                          \
		.element_name = STRINGIFY(field_name_), .type = type_,                                               \
			.element_name_len = sizeof(STRINGIFY(field_name_)) - 1,                    \
						   .offset = offsetof(struct_, field_name_),       \
						   .size = sizeof(((struct_ *)0)->field_name_),    \
			}

int lua_zephyr_decode(lua_State *L, const struct lua_zephyr_table_descr *desc, void *struct_ptr,
		      size_t desc_size, int table_index);

int lua_zephyr_encode(lua_State *L, const struct lua_zephyr_table_descr *desc,
		      const void *struct_ptr, size_t desc_size);

#endif /* _LUA_ZEPHYR_CODEC_H */
