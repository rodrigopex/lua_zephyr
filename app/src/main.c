#include <zephyr/kernel.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <lua_zephyr/utils.h>

#include "sample01_lua_script.h"

int lua_hello_world_setup(lua_State *L)
{
	printk("Pre-lua vm setup for hello world lua by the user\n");

	return 0;
}

int main(int argc, char *argv[])
{
	lua_State *L = luaL_newstate();

	lua_register_zephyr_api(L);

	if (luaL_dostring(L, sample01_lua_script) != LUA_OK) {
		printk("Error: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	lua_close(L);

	k_sleep(K_FOREVER);
	return 0;
}
