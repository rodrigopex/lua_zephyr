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
 *   LUA_PB_DESCR_DEFINE(msg_acc_data);
 *   ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL,
 *                    LUA_PB_DESCR(msg_acc_data), ...);
 *
 * For nested MESSAGE fields, define submessage descriptors first and
 * set LUA_PB_SUBMSG_<fieldname> before calling LUA_PB_DESCR_DEFINE:
 *
 *   LUA_PB_DESCR_DEFINE(msg_acc_data);
 *   #define LUA_PB_SUBMSG_offset msg_acc_data_lua_fields
 *   LUA_PB_DESCR_DEFINE(msg_sensor_config);
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
#define LUA_PB_GEN_MESSAGE(s, f)         LUA_MSG_FIELD_OBJECT(s, f, LUA_PB_SUBMSG_##f)

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
 * @brief Define fields array + descriptor struct from a nanopb FIELDLIST.
 *
 * Generates <_name>_lua_fields[] and <_name>_lua_descr from
 * <_name>_FIELDLIST. Use LUA_PB_DESCR(_name) to reference the
 * descriptor in ZBUS_CHAN_DEFINE user_data.
 *
 * @param _name  The struct tag name (e.g. msg_acc_data).
 */
#define LUA_PB_DESCR_DEFINE(_name)                                             \
	static const struct lua_msg_field_descr _name##_lua_fields[] = {       \
		_name##_FIELDLIST(LUA_PB_GEN_FIELD, struct _name)              \
	};                                                                     \
	LUA_ZBUS_MSG_DESCR_DEFINE(                                             \
		_name##_lua_descr, struct _name, _name##_lua_fields)

/**
 * @brief Reference descriptor for use in ZBUS_CHAN_DEFINE user_data.
 *
 * @param _name  The nanopb type name (e.g. msg_acc_data).
 */
#define LUA_PB_DESCR(_name) ((void *)&_name##_lua_descr)

/* clang-format on */

#endif /* LUA_MSG_DESCR_PB_H */
