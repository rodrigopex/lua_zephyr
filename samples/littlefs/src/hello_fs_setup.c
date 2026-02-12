/**
 * @file hello_fs_setup.c
 * @brief Setup hook for the hello_fs LittleFS Lua thread.
 *
 * Loads the zephyr and fs libraries into the Lua state before the
 * FS-backed script executes.
 */

#include <lauxlib.h>
#include <lualib.h>
#include <luaz_utils.h>
#include <luaz_fs.h>

/** @brief Setup hook: load zephyr + fs libraries for the FS-backed thread. */
int lfs_hello_fs_lua_lua_setup(lua_State *L)
{
	LUA_REQUIRE(zephyr);
	LUA_REQUIRE(base);
	LUA_REQUIRE(fs);
	return 0;
}
