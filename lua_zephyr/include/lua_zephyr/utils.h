#ifndef _LUA_ZEPHYR_UTILS_H
#define _LUA_ZEPHYR_UTILS_H

#include <lua.h>
#include <zephyr/kernel.h>

/* clang-format off */
#define LUA_REQUIRE(_lib)                             \
	do {                                              \
		luaL_requiref(L, #_lib, luaopen_##_lib, 1);  \
		lua_pop(L, 1);                                \
	} while (0)
/* clang-format on */

void *lua_zephyr_allocator(void *ud, void *ptr, size_t osize, size_t nsize);

int luaopen_zephyr(lua_State *L);

#endif /* _LUA_ZEPHYR_UTILS_H */
