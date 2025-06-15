#ifndef _LUA_ZEPHYR_UTILS_H
#define _LUA_ZEPHYR_UTILS_H

#include <lua.h>
#include <zephyr/kernel.h>

/* clang-format off */
#define LUA_REQUIRE(_lib)                             \
	do {                                              \
		luaL_requiref(L, #_lib, luaopen_##_lib, 1);   \
		lua_pop(L, 1);                                \
	} while (0)

#define LUA_TABLE_SET(_k_string, _v_lua_type_name, _v)   \
	lua_pushstring(L, _k_string);                        \
	lua_push##_v_lua_type_name(L, _v);                   \
	lua_settable(L, -3);

#define LUA_TABLE_GET(_k_string, _var_lua_type_name)   \
	({                                                 \
		lua_getfield(L, 2, _k_string);                 \
		lua_to##_var_lua_type_name(L, -1);             \
	})
/* clang-format on */

void *lua_zephyr_allocator(void *ud, void *ptr, size_t osize, size_t nsize);

int luaopen_zephyr(lua_State *L);

#endif /* _LUA_ZEPHYR_UTILS_H */
