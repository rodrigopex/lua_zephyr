/**
 * @file main.c
 * @brief Heavy sample: Lua thread with the string library enabled.
 */

#include <zephyr/kernel.h>

#include <lauxlib.h>
#include <lualib.h>
#include <lua_zephyr/luaz_utils.h>

/** @brief Setup hook for the heavy Lua thread (loads string + zephyr libs). */
int heavy_lua_setup(lua_State *L)
{
	LUA_REQUIRE(string);
	LUA_REQUIRE(zephyr);
	return 0;
}
