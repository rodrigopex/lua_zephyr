#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <lua_zephyr/codec.h>
#include <lua_zephyr/utils.h>

#define CAST_STRUCT(ptr_, desc_) ((const uint8_t *)(ptr_) + (desc_)->offset)

#define CAST_STRUCT_MEMBER_PRIM(type_, ptr_, desc_) ((type_)(CAST_STRUCT(ptr_, desc_)))

#define CAST_STRUCT_MEMBER_STR(ptr_, desc_) (const char **)CAST_STRUCT(ptr_, desc_)

static int struct_member_to_lua(lua_State *L, const struct lua_object_descriptor *desc,
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
		char *str_ptr = *CAST_STRUCT_MEMBER_STR(struct_ptr, desc);
		lua_pushlstring(L, str_ptr, strlen(str_ptr));
		break;
	case LUA_CODEC_VALUE_TYPE_INTEGER:
		lua_pushinteger(L, *CAST_STRUCT_MEMBER_PRIM(int32_t *, struct_ptr, desc));
		break;
	default:
		return -EINVAL;
	}

	lua_setfield(L, -2, desc->element_name);
	return 0;
}

static int lua_field_to_struct_member(lua_State *L, const char *key, void *struct_ptr,
				      const struct lua_object_descriptor *desc)
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
		strncpy(CAST_STRUCT_MEMBER_STR(struct_ptr, desc), str, str_len + 1);
		break;
	case LUA_CODEC_VALUE_TYPE_INTEGER:
		if (!lua_isinteger(L, -1)) {
			return -EINVAL;
		}
		*CAST_STRUCT_MEMBER_PRIM(int32_t *, struct_ptr, desc) = lua_tointeger(L, -1);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int find_member_by_name(const struct lua_object_descriptor *desc, size_t desc_size,
			       const char *name)
{
	for (size_t i = 0; i < desc_size; i++) {
		if (strncmp(desc[i].element_name, name, desc[i].element_name_len) == 0) {
			return i;
		}
	}
	return -ENOENT;
}

int lua_table_to_struct(lua_State *L, const struct lua_object_descriptor *desc, void *struct_ptr,
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

int struct_to_lua_table(lua_State *L, const struct lua_object_descriptor *desc,
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
