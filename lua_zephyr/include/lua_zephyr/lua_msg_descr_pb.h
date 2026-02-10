/**
 * @file lua_msg_descr_pb.h
 * @brief Bridge between nanopb FIELDLIST X-macros and lua_msg_field_descr arrays.
 *
 * Automatically generates lua_msg_field_descr arrays from nanopb-generated
 * FIELDLIST macros, making the .proto file the single source of truth for
 * both protobuf serialization and Lua table conversion.
 *
 * Usage:
 *   #include <lua_zephyr/lua_msg_descr_pb.h>
 *   #include "channels.pb.h"
 *
 *   ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL,
 *                    LUA_PB_DESCR(msg_acc_data), ...);
 *
 * For nested MESSAGE fields, set LUA_PB_SUBMSG_<fieldname> before use:
 *
 *   #define LUA_PB_SUBMSG_offset LUA_PB_FIELDS(msg_acc_data)
 *   ZBUS_CHAN_DEFINE(chan_sensor_config, struct msg_sensor_config, NULL,
 *                    LUA_PB_DESCR(msg_sensor_config), ...);
 *   #undef LUA_PB_SUBMSG_offset
 *
 * Limitations:
 *   - ONEOF fields not supported (fieldname is a tuple)
 *   - REPEATED/FIXARRAY fields not supported
 *   - Only STATIC allocation type supported
 */

#ifndef LUA_MSG_DESCR_PB_H
#define LUA_MSG_DESCR_PB_H

#include <lua_zephyr/lua_msg_descr.h>

/* clang-format off */

/**
 * @brief Type-mapping macros: nanopb ltype -> LUA_MSG_FIELD invocation.
 *
 * Each LUA_PB_GEN_<ltype> maps a nanopb logical type to the appropriate
 * lua_msg_field_descr entry.
 */
#define LUA_PB_GEN_BOOL(s, f)            LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_BOOL)
#define LUA_PB_GEN_INT32(s, f)           LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_INT)
#define LUA_PB_GEN_SINT32(s, f)          LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_INT)
#define LUA_PB_GEN_SFIXED32(s, f)        LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_INT)
#define LUA_PB_GEN_INT64(s, f)           LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_INT)
#define LUA_PB_GEN_SINT64(s, f)          LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_INT)
#define LUA_PB_GEN_SFIXED64(s, f)        LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_INT)
#define LUA_PB_GEN_UINT32(s, f)          LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_UINT)
#define LUA_PB_GEN_FIXED32(s, f)         LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_UINT)
#define LUA_PB_GEN_UINT64(s, f)          LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_UINT)
#define LUA_PB_GEN_FIXED64(s, f)         LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_UINT)
#define LUA_PB_GEN_FLOAT(s, f)           LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_NUMBER)
#define LUA_PB_GEN_DOUBLE(s, f)          LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_NUMBER)
#define LUA_PB_GEN_STRING(s, f)          LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_STRING_BUF)
#define LUA_PB_GEN_ENUM(s, f)            LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_INT)
#define LUA_PB_GEN_UENUM(s, f)           LUA_MSG_FIELD(s, f, LUA_MSG_TYPE_UINT)
#define LUA_PB_GEN_MESSAGE(s, f)                                       \
	{                                                              \
		.field_name = #f,                                      \
		.type = LUA_MSG_TYPE_OBJECT,                           \
		.offset = offsetof(s, f),                              \
		.size = sizeof(((s *)0)->f),                           \
		.sub_fields = LUA_PB_SUBMSG_##f,                       \
		.sub_field_count = sizeof(LUA_PB_SUBMSG_##f)           \
			/ sizeof(struct lua_msg_field_descr),           \
	}

/**
 * @brief X-macro callback: dispatches on nanopb ltype via token pasting.
 *
 * Called by the nanopb-generated FIELDLIST macro for each field.
 * Ignores atype, htype, and tag; dispatches solely on ltype.
 *
 * @param structtype  The C struct type (passed as the accumulator 'a').
 * @param atype       Allocation type (STATIC, POINTER, CALLBACK) - ignored.
 * @param htype       Handling type (REQUIRED, OPTIONAL, etc.) - ignored.
 * @param ltype       Logical type (INT32, UINT32, STRING, etc.) - used for dispatch.
 * @param fieldname   The C struct field name.
 * @param tag         Proto field tag number - ignored.
 */
#define LUA_PB_GEN_FIELD(structtype, atype, htype, ltype, fieldname, tag) \
	LUA_PB_GEN_##ltype(structtype, fieldname),

/**
 * @brief X-macro callback for submessage fields (avoids preprocessor
 *        blue-painting when nested inside LUA_PB_GEN_FIELD expansion).
 */
#define LUA_PB_GEN_SUB_FIELD(structtype, atype, htype, ltype, fieldname, tag) \
	LUA_PB_GEN_##ltype(structtype, fieldname),

/**
 * @brief Compound literal for a nanopb fields array.
 *
 * Use to define LUA_PB_SUBMSG_<field> for nested MESSAGE fields.
 *
 * @param _name  The struct tag name (e.g. msg_acc_data).
 */
#define LUA_PB_FIELDS(_name)                                                   \
	((const struct lua_msg_field_descr[]){                                 \
		_name##_FIELDLIST(LUA_PB_GEN_SUB_FIELD, struct _name)          \
	})

/**
 * @brief Create a compound-literal descriptor from a nanopb FIELDLIST.
 *
 * Returns (void *) suitable for ZBUS_CHAN_DEFINE user_data.
 *
 * @param _name  The struct tag name (e.g. msg_acc_data).
 */
#define LUA_PB_DESCR_DEFINE(_name)                                                    \
	const struct lua_msg_descr CONCAT(_name, _descr) = {                                \
		.fields = (const struct lua_msg_field_descr[]){                \
			_name##_FIELDLIST(LUA_PB_GEN_FIELD, struct _name)      \
		},                                                             \
		.field_count = sizeof((const struct lua_msg_field_descr[]){    \
			_name##_FIELDLIST(LUA_PB_GEN_FIELD, struct _name)      \
		}) / sizeof(struct lua_msg_field_descr),                       \
		.msg_size = sizeof(struct _name),                              \
	}

#define LUA_PB_DESCR_REF(_name) ((void*)&CONCAT(_name, _descr))
/* clang-format on */

#endif /* LUA_MSG_DESCR_PB_H */
