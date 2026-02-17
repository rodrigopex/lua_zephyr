/**
 * @file main.c
 * @brief Hello-world sample: runs a Lua script from both main and a Lua thread.
 */

#include <zephyr/kernel.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <luaz_utils.h>

#include "sample01_lua_script.h"

/** @brief Run the sample01 Lua script using the default Lua allocator. */
int main(int argc, char *argv[])
{
	lua_State *L = luaL_newstate();

	luaz_openlibs(L);

	if (luaL_dostring(L, sample01_lua_script) != LUA_OK) {
		printk("Error: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	lua_close(L);

	k_sleep(K_FOREVER);
	return 0;
}
