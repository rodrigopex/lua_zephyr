/**
 * @file lua_msg_descr_pb.h
 * @brief Bridge between nanopb FIELDLIST X-macros and lua_msg_field_descr arrays.
 *
 * Automatically generates lua_msg_field_descr arrays from nanopb-generated
 * FIELDLIST macros, making the .proto file the single source of truth for
 * both protobuf serialization and Lua table conversion.
 *
 * Usage (without --c-style nanopb option):
 *   #include <lua_zephyr/lua_msg_descr_pb.h>
 *   #include "channels.pb.h"
 *
 *   LUA_PB_DESCR(MsgAccData);
 *
 * For nested MESSAGE fields, define submessage descriptors first and
 * set LUA_PB_SUBMSG_<fieldname> before calling LUA_PB_DESCR:
 *
 *   LUA_PB_DESCR(MsgAccData);
 *   #define LUA_PB_SUBMSG_offset MsgAccData_lua_fields
 *   LUA_PB_DESCR(MsgSensorConfig);
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
 * @brief Generate a lua_msg_field_descr array from a nanopb FIELDLIST.
 *
 * Single-argument macro: derives the FIELDLIST name and struct type from
 * the nanopb typedef name. Requires nanopb without --c-style so that
 * FIELDLIST macros use the lowercase typedef name as prefix.
 *
 * @param _name  The nanopb typedef name (e.g. msg_acc_data).
 *               Generates array <_name>_lua_fields from <_name>_FIELDLIST.
 */
#define LUA_PB_DESCR(_name)                                                    \
	static const struct lua_msg_field_descr _name##_lua_fields[] = {       \
		_name##_FIELDLIST(LUA_PB_GEN_FIELD, _name)                     \
	}

/**
 * @brief Create an inline zbus user_data descriptor from a nanopb typedef.
 *
 * Convenience wrapper combining LUA_ZBUS_MSG_DESCR with the naming
 * convention from LUA_PB_DESCR.
 *
 * @param _name  The nanopb typedef name (e.g. msg_acc_data).
 */
#define LUA_PB_ZBUS_MSG_DESCR(_name)                                           \
	LUA_ZBUS_MSG_DESCR(_name, _name##_lua_fields)

/* clang-format on */

#endif /* LUA_MSG_DESCR_PB_H */
