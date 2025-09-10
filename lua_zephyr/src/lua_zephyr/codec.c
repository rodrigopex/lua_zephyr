#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <lua_zephyr/codec.h>
#include <lua_zephyr/utils.h>

#define CAST_STRUCT(ptr_, desc_)                    ((const uint8_t *)(ptr_) + (desc_)->offset)
#define CAST_STRUCT_MEMBER_PRIM(type_, ptr_, desc_) ((type_)(CAST_STRUCT(ptr_, desc_)))
#define CAST_STRUCT_MEMBER_STR(ptr_, desc_)         (char **)CAST_STRUCT(ptr_, desc_)
#define CAST_STRUCT_MEMBER_LEN(ptr_, desc_)                                                        \
	(size_t *)((const uint8_t *)(ptr_) + (desc_)->arr_len_offset)
#define CAST_STRUCT_MEMBER_ARR(type_, ptr_, desc_, idx_)                                           \
	(type_)((const uint8_t *)(ptr_) + (desc_)->offset + ((idx_) * sizeof(type_)))

static void handle_array_encoding(lua_State *L, const struct lua_zephyr_table_descr *desc,
				  const void *struct_ptr)
{
	size_t array_size = *CAST_STRUCT_MEMBER_PRIM(size_t *, struct_ptr, desc);
	lua_createtable(L, array_size, 0);

	for (size_t i = 0; i < array_size; i++) {
		/* Indexes in Lua start at 1 */
		lua_pushinteger(L, i + 1);

		switch (desc->array_element_type) {
		case LUA_CODEC_VALUE_TYPE_BOOLEAN:
			lua_pushboolean(L, *CAST_STRUCT_MEMBER_ARR(bool *, struct_ptr, desc, i));
			break;
		case LUA_CODEC_VALUE_TYPE_NUMBER:
			lua_pushnumber(L, *CAST_STRUCT_MEMBER_ARR(float *, struct_ptr, desc, i));
			break;
		case LUA_CODEC_VALUE_TYPE_STRING: {
			const char *str_ptr = *CAST_STRUCT_MEMBER_STR(struct_ptr, desc);
			lua_pushlstring(L, str_ptr, strlen(str_ptr));
		} break;
		case LUA_CODEC_VALUE_TYPE_INTEGER:
			lua_pushinteger(L, *CAST_STRUCT_MEMBER_ARR(int32_t *, struct_ptr, desc, i));
			break;
		default:
		case LUA_CODEC_VALUE_TYPE_NIL:
			/* Unsupported type, push nil */
			lua_pushnil(L);
		}
	}
}

static int handle_array_decoding(lua_State *L, const struct lua_zephyr_table_descr *desc,
				 void *struct_ptr)
{
	if (!lua_istable(L, -1)) {
		return -EINVAL;
	}

	size_t array_size = lua_rawlen(L, -1);
	*CAST_STRUCT_MEMBER_LEN(struct_ptr, desc) = array_size;

	for (size_t i = 0; i < array_size; i++) {
		/* Indexes in Lua start at 1 */
		lua_pushinteger(L, i + 1);
		lua_gettable(L, -2);

		switch (desc->array_element_type) {
		case LUA_CODEC_VALUE_TYPE_BOOLEAN:
			if (!lua_isboolean(L, -1)) {
				return -EINVAL;
			}

			*CAST_STRUCT_MEMBER_ARR(bool *, struct_ptr, desc, i) = lua_toboolean(L, -1);
			break;
		case LUA_CODEC_VALUE_TYPE_NUMBER:
			if (!lua_isnumber(L, -1)) {
				return -EINVAL;
			}

			*CAST_STRUCT_MEMBER_ARR(float *, struct_ptr, desc, i) = lua_tonumber(L, -1);
			break;
		case LUA_CODEC_VALUE_TYPE_STRING:
			if (!lua_isstring(L, -1)) {
				return -EINVAL;
			}

			size_t str_len;
			const char *str = lua_tolstring(L, -1, &str_len);
			if (str_len >= desc->size) {
				return -EINVAL;
			}

			strncpy((char *)CAST_STRUCT_MEMBER_STR(struct_ptr, desc), str, str_len + 1);
			break;
		case LUA_CODEC_VALUE_TYPE_INTEGER:
			if (!lua_isinteger(L, -1)) {
				return -EINVAL;
			}

			*CAST_STRUCT_MEMBER_ARR(int32_t *, struct_ptr, desc, i) =
				lua_tointeger(L, -1);
			break;
		default:
		case LUA_CODEC_VALUE_TYPE_NIL:
			/* Unsupported type, skip */
			break;
		}

		lua_pop(L, 1);
	}

	return 0;
}

static int struct_member_to_lua(lua_State *L, const struct lua_zephyr_table_descr *desc,
				const void *struct_ptr)
{
	switch (desc->type) {
	case LUA_CODEC_VALUE_TYPE_NIL:
		lua_pushnil(L);
		break;
	case LUA_CODEC_VALUE_TYPE_BOOLEAN:
		lua_pushboolean(L, *CAST_STRUCT_MEMBER_PRIM(bool *, struct_ptr, desc));
		break;
	case LUA_CODEC_VALUE_TYPE_NUMBER:
		lua_pushnumber(L, *CAST_STRUCT_MEMBER_PRIM(float *, struct_ptr, desc));
		break;
	case LUA_CODEC_VALUE_TYPE_STRING:
		const char *str_ptr = *CAST_STRUCT_MEMBER_STR(struct_ptr, desc);
		lua_pushlstring(L, str_ptr, strlen(str_ptr));
		break;
	case LUA_CODEC_VALUE_TYPE_INTEGER:
		lua_pushinteger(L, *CAST_STRUCT_MEMBER_PRIM(int32_t *, struct_ptr, desc));
		break;
	case LUA_CODEC_VALUE_TYPE_ARRAY:
		(void)handle_array_encoding(L, desc, struct_ptr);
		break;
	default:
		return -EINVAL;
	}

	lua_setfield(L, -2, desc->element_name);
	return 0;
}

static int lua_field_to_struct_member(lua_State *L, const char *key, void *struct_ptr,
				      const struct lua_zephyr_table_descr *desc)
{
	switch (desc->type) {
	case LUA_CODEC_VALUE_TYPE_NIL:
		break;
	case LUA_CODEC_VALUE_TYPE_BOOLEAN:
		if (!lua_isboolean(L, -1)) {
			return -EINVAL;
		}

		*CAST_STRUCT_MEMBER_PRIM(bool *, struct_ptr, desc) = lua_toboolean(L, -1);
		break;
	case LUA_CODEC_VALUE_TYPE_NUMBER:
		if (!lua_isnumber(L, -1)) {
			return -EINVAL;
		}

		*CAST_STRUCT_MEMBER_PRIM(float *, struct_ptr, desc) = lua_tonumber(L, -1);
		break;
	case LUA_CODEC_VALUE_TYPE_STRING:
		if (!lua_isstring(L, -1)) {
			return -EINVAL;
		}

		size_t str_len;
		const char *str = lua_tolstring(L, -1, &str_len);
		if (str_len >= desc->size) {
			return -EINVAL;
		}

		strncpy((char *)CAST_STRUCT_MEMBER_STR(struct_ptr, desc), str, str_len + 1);
		break;
	case LUA_CODEC_VALUE_TYPE_INTEGER:
		if (!lua_isinteger(L, -1)) {
			return -EINVAL;
		}

		*CAST_STRUCT_MEMBER_PRIM(int32_t *, struct_ptr, desc) = lua_tointeger(L, -1);
		break;
	case LUA_CODEC_VALUE_TYPE_ARRAY:
		return handle_array_decoding(L, desc, struct_ptr);
	default:
		return -EINVAL;
	}

	return 0;
}

static int find_member_by_name(const struct lua_zephyr_table_descr *desc, size_t desc_size,
			       const char *name)
{
	for (size_t i = 0; i < desc_size; i++) {
		if (strncmp(desc[i].element_name, name, desc[i].element_name_len) == 0) {
			return i;
		}
	}
	return -ENOENT;
}

int lua_zephyr_decode(lua_State *L, const struct lua_zephyr_table_descr *desc, void *struct_ptr,
		      size_t desc_size, int table_index)
{
	int err;

	if ((L == NULL) || (desc == NULL) || (struct_ptr == NULL)) {
		return -EINVAL;
	}

	lua_pushnil(L);
	while (lua_next(L, table_index) != 0) {
		const char *key = lua_tostring(L, -2);
		if (key == NULL) {
			return -EINVAL;
		}

		int member_idx = find_member_by_name(desc, desc_size, key);
		if (member_idx == -ENOENT) {
			lua_pop(L, 2);
			return -EINVAL;
		}

		err = lua_field_to_struct_member(L, key, struct_ptr, &desc[member_idx]);
		if (err < 0) {
			lua_pop(L, 2);
			return err;
		}

		lua_pop(L, 1);
	}

	return 0;
}

int lua_zephyr_encode(lua_State *L, const struct lua_zephyr_table_descr *desc,
		      const void *struct_ptr, size_t desc_size)
{
	int err;

	if ((L == NULL) || (desc == NULL) || (struct_ptr == NULL)) {
		return -EINVAL;
	}

	lua_newtable(L);
	for (size_t i = 0; i < desc_size; i++) {
		err = struct_member_to_lua(L, &desc[i], struct_ptr);
		if (err < 0) {
			return err;
		}
	}

	return 0;
}
