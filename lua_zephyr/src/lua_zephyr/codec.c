#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <lua_zephyr/codec.h>
#include <lua_zephyr/utils.h>

#define CAST_STRUCT(ptr_, desc_) ((const uint8_t *)(ptr_) + (desc_)->offset)

#define CAST_STRUCT_MEMBER_PRIM(type_, ptr_, desc_) ((type_)(CAST_STRUCT(ptr_, desc_)))

#define CAST_STRUCT_MEMBER_STR(ptr_, desc_) *(const char **)CAST_STRUCT(ptr_, desc_)

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
		char *str_ptr = CAST_STRUCT_MEMBER_STR(struct_ptr, desc);
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

int lua_table_to_struct(lua_State *L, const struct lua_object_descriptor *desc, void *struct_ptr)
{
	return 0;
}

int struct_to_lua_table(lua_State *L, const struct lua_object_descriptor *desc,
			const void *struct_ptr, size_t desc_size)
{
	int ret;

	if ((L == NULL) || (desc == NULL) || (struct_ptr == NULL)) {
		return -EINVAL;
	}

	lua_newtable(L);
	for (size_t i = 0; i < desc_size; i++) {
		ret = struct_member_to_lua(L, &desc[i], struct_ptr);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}
