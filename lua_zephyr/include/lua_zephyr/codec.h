/**
 * @file codec.h
 * @brief Lua Zephyr codec header file
 *
 * This file contains the definitions and function prototypes for encoding and decoding
 * Lua tables to and from C structures in the Zephyr environment.
 */

#ifndef _LUA_ZEPHYR_CODEC_H
#define _LUA_ZEPHYR_CODEC_H

#include <lua.h>
#include <zephyr/sys/util.h>

/**
 * @brief Type used to describe the type of a value in a Lua table.
 *
 * Can be nested for arrays, i.e. an array of integers.
 */
enum lua_codec_value_type {
	LUA_CODEC_VALUE_TYPE_NIL,     /**< No value / unsupported type */
	LUA_CODEC_VALUE_TYPE_BOOLEAN, /**< Boolean value */
	LUA_CODEC_VALUE_TYPE_NUMBER,  /**< Floating point number */
	LUA_CODEC_VALUE_TYPE_STRING,  /**< String value */
	LUA_CODEC_VALUE_TYPE_INTEGER, /**< Integer value */
	LUA_CODEC_VALUE_TYPE_ARRAY,   /**< Array of values */
};

/**
 * @brief Wrapper type to hold metadata for encoding/decoding operations.
 */
struct user_data_wrapper {
	const struct lua_zephyr_table_descr
		*desc; /**< Array of element descriptors which maps a Lua table to a C structure */
	size_t desc_size; /**< Number of elements in the descriptor array */
};

/**
 * @brief Descriptor for a single element in a Lua table and its corresponding C structure member.
 */
struct lua_zephyr_table_descr {
	const char *element_name;       /**< Name of the element in the Lua table */
	enum lua_codec_value_type type; /**< Type of the element */
	size_t element_name_len;        /**< Length of the element name */
	size_t offset;         /**< Offset of the corresponding member in the C structure */
	size_t size;           /**< Size of the corresponding member in the C structure */
	size_t arr_len_offset; /**< Offset of the member that holds the length of the array (only
				  for arrays) */
	enum lua_codec_value_type
		array_element_type; /**< Type of the elements in the array (only for arrays) */
};

/**
 * @brief Macro to calculate and embed the size of a table descriptor in a user_data_wrapper.
 */
#define LUA_ZEPHYR_WRAPPER_DESC(desc_)                                                             \
	const struct user_data_wrapper ud_##desc_ = {                                              \
		.desc = desc_,                                                                     \
		.desc_size = ARRAY_SIZE(desc_),                                                    \
	}

/**
 * @brief Macro to define a field descriptor for a primitive type in a C structure.
 *
 * @param struct_ The C structure type.
 * @param field_name_ The name of the field in the structure.
 * @param type_ The lua_codec_value_type of the field.
 */
#define LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct_, field_name_, type_)                               \
	{                                                                                          \
		.element_name = STRINGIFY(field_name_), .type = type_,                                               \
			.element_name_len = sizeof(STRINGIFY(field_name_)) - 1,                    \
						   .offset = offsetof(struct_, field_name_),       \
						   .size = sizeof(((struct_ *)0)->field_name_),    \
			}

/**
 * @brief Macro to define a field descriptor for an array type in a C structure.
 *
 * @param struct_ The C structure type.
 * @param field_name_ The name of the array field in the structure.
 * @param type_ The lua_codec_value_type of the elements in the array.
 * @param size_member_ The name of the member that holds the length of the array.
 */
#define LUA_TABLE_FIELD_DESCRIPTOR_ARRAY(struct_, field_name_, type_, size_member_)                \
	{                                                                                          \
		.element_name = STRINGIFY(field_name_), .type = LUA_CODEC_VALUE_TYPE_ARRAY,                          \
			.element_name_len =                                                        \
				sizeof(STRINGIFY(field_name_)) - 1,                                \
				       .offset = offsetof(struct_, field_name_),                   \
				       .size = sizeof(((struct_ *)0)->field_name_),                \
				       .arr_len_offset = offsetof(struct_, size_member_),          \
				       .array_element_type = type_,                                \
			}

/**
 * @brief Decode a Lua table at the given index into a C structure based on the provided descriptor.
 *
 * @param L The Lua state.
 * @param desc The descriptor array mapping Lua table elements to C structure members.
 * @param struct_ptr Pointer to the C structure to populate.
 * @param desc_size Number of elements in the descriptor array.
 * @param table_index Index of the Lua table on the stack.
 * @return 0 on success, -ERRNO on failure.
 */
int lua_zephyr_decode(lua_State *L, const struct lua_zephyr_table_descr *desc, void *struct_ptr,
		      size_t desc_size, int table_index);

/**
 * @brief Encode a C structure into a Lua table based on the provided descriptor.
 *
 * @param L The Lua state.
 * @param desc The descriptor array mapping Lua table elements to C structure members.
 * @param struct_ptr Pointer to the C structure to encode.
 * @param desc_size Number of elements in the descriptor array.
 * @return 0 on success, -ERRNO on failure.
 */
int lua_zephyr_encode(lua_State *L, const struct lua_zephyr_table_descr *desc,
		      const void *struct_ptr, size_t desc_size);

#endif /* _LUA_ZEPHYR_CODEC_H */
