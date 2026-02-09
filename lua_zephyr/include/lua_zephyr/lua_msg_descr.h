/**
 * @file lua_msg_descr.h
 * @brief Descriptor-based Lua <-> C struct conversion for zbus messages.
 *
 * Provides a declarative way to define how zbus message structs map to Lua
 * tables. Descriptors are stored as zbus channel user_data for O(1) lookup.
 * Pass LUA_ZBUS_MSG_DESCR(type, fields) as user_data to ZBUS_CHAN_DEFINE.
 */

#ifndef LUA_MSG_DESCR_H
#define LUA_MSG_DESCR_H

#include <lua.h>
#include <stddef.h>
#include <stdint.h>
#include <zephyr/zbus/zbus.h>

/** @brief Field type for Lua message descriptors. */
enum lua_msg_field_type {
	LUA_MSG_TYPE_INT,    /* signed int (1/2/4/8 bytes) -> lua_pushinteger */
	LUA_MSG_TYPE_UINT,   /* unsigned int (1/2/4/8 bytes) -> lua_pushinteger */
	LUA_MSG_TYPE_NUMBER, /* float/double -> lua_pushnumber */
	LUA_MSG_TYPE_STRING,     /* const char* -> lua_pushstring */
	LUA_MSG_TYPE_STRING_BUF, /* inline char[] -> lua_pushstring */
	LUA_MSG_TYPE_BOOL,       /* bool -> lua_pushboolean */
	LUA_MSG_TYPE_OBJECT,     /* nested struct -> nested Lua table */
};

/**
 * @brief Descriptor for a single field in a message struct.
 *
 * Each descriptor maps a C struct field to a named Lua table entry.
 * For nested structs (LUA_MSG_TYPE_OBJECT), sub_fields and sub_field_count
 * point to the nested field descriptor array.
 */
struct lua_msg_field_descr {
	const char *field_name;
	enum lua_msg_field_type type;
	uint16_t offset;
	uint8_t size;
	const struct lua_msg_field_descr *sub_fields;
	size_t sub_field_count;
};

/**
 * @brief Top-level descriptor for a message struct.
 *
 * Stored as zbus channel user_data for O(1) lookup at runtime.
 */
struct lua_msg_descr {
	const struct lua_msg_field_descr *fields;
	size_t field_count;
	size_t msg_size;
};

/* clang-format off */

/**
 * @brief Define a primitive field descriptor.
 *
 * @param _struct  The C struct type.
 * @param _field   The field name.
 * @param _type    The lua_msg_field_type enum value.
 */
#define LUA_MSG_FIELD(_struct, _field, _type)                         \
	{                                                             \
		.field_name = #_field,                                \
		.type = (_type),                                      \
		.offset = offsetof(_struct, _field),                  \
		.size = sizeof(((_struct *)0)->_field),                \
		.sub_fields = NULL,                                   \
		.sub_field_count = 0,                                 \
	}

/**
 * @brief Define a nested struct (object) field descriptor.
 *
 * @param _struct      The parent C struct type.
 * @param _field       The field name in the parent struct.
 * @param _sub_fields  Array of lua_msg_field_descr for the nested struct.
 */
#define LUA_MSG_FIELD_OBJECT(_struct, _field, _sub_fields)            \
	{                                                             \
		.field_name = #_field,                                \
		.type = LUA_MSG_TYPE_OBJECT,                          \
		.offset = offsetof(_struct, _field),                  \
		.size = sizeof(((_struct *)0)->_field),                \
		.sub_fields = (_sub_fields),                          \
		.sub_field_count = ARRAY_SIZE(_sub_fields),           \
	}

/**
 * @brief Define a standalone message descriptor.
 *
 * Creates a static const lua_msg_descr. Use this when you need a descriptor
 * without immediately binding it to a channel.
 *
 * @param _name    Unique C identifier for this descriptor.
 * @param _struct  The C struct type for the message.
 * @param _fields  Array of lua_msg_field_descr.
 */
#define LUA_ZBUS_MSG_DESCR_DEFINE(_name, _struct, _fields)            \
	static const struct lua_msg_descr _name __used = {            \
		.fields = (_fields),                                  \
		.field_count = ARRAY_SIZE(_fields),                   \
		.msg_size = sizeof(_struct),                          \
	}

/**
 * @brief Create an inline descriptor pointer (compound literal).
 *
 * Returns a (void *) to a compound-literal lua_msg_descr, suitable
 * for passing directly as the _user_data argument of ZBUS_CHAN_DEFINE.
 * No separate LUA_ZBUS_CHAN_DESCR_DEFINE step is needed.
 *
 * Usage:
 *   ZBUS_CHAN_DEFINE(chan, struct msg, NULL,
 *                   LUA_ZBUS_MSG_DESCR(struct msg, fields),
 *                   ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.x = 0));
 *
 * @param _struct  The C struct type for the message.
 * @param _fields  Array of lua_msg_field_descr.
 */
#define LUA_ZBUS_MSG_DESCR(_struct, _fields)                          \
	((void *)&(const struct lua_msg_descr){                       \
		.fields = (_fields),                                  \
		.field_count = ARRAY_SIZE(_fields),                   \
		.msg_size = sizeof(_struct),                          \
	})

/* clang-format on */

/**
 * @brief Encode C struct fields into a Lua table (push to stack).
 *
 * @param L            Lua state.
 * @param fields       Array of field descriptors.
 * @param field_count  Number of fields.
 * @param base         Base pointer to the C struct data.
 */
void lua_msg_descr_to_table(lua_State *L, const struct lua_msg_field_descr *fields,
			    size_t field_count, const void *base);

/**
 * @brief Decode a Lua table into C struct fields.
 *
 * @param L            Lua state.
 * @param fields       Array of field descriptors.
 * @param field_count  Number of fields.
 * @param base         Base pointer to the C struct to populate.
 * @param table_idx    Absolute Lua stack index of the source table.
 */
void lua_msg_descr_from_table(lua_State *L, const struct lua_msg_field_descr *fields,
			      size_t field_count, void *base, int table_idx);

#endif /* LUA_MSG_DESCR_H */
