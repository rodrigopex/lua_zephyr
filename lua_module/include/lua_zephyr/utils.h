#ifndef _LUA_ZEPHYR_UTILS_H
#define _LUA_ZEPHYR_UTILS_H

#include <lua.h>
#include <zephyr/kernel.h>

void *lua_zephyr_allocator(void *ud, void *ptr, size_t osize, size_t nsize);

int lua_register_zephyr_api(lua_State *L);

#endif /* _LUA_ZEPHYR_UTILS_H */
