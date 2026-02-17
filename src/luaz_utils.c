/**
 * @file luaz_utils.c
 * @brief Lua-Zephyr core: allocator, kernel API wrappers, and POSIX stubs.
 *
 * Implements the sys_heap-backed Lua allocator and the `zephyr` Lua library
 * which exposes msleep, printk, and LOG_* to Lua scripts.
 */

#include "lua.h"
#include "luaz_utils.h"

#include <lauxlib.h>
#include <lualib.h>
#include <luaz_zbus.h>
#include <sys/times.h>
#include <time.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_LUA_FS
#include <luaz_fs.h>
#endif

LOG_MODULE_REGISTER(lua_zephyr, CONFIG_LUA_ZEPHYR_LOG_LEVEL);

/**
 * @brief The custom allocator that Lua will use to dynamically allocate all
relevant data in the context
 *
 * @param ud User data, a reference to the custom heap region the allocator will
use
 * @param ptr A pointer to the object that's going to be allocated/rellocated
 * @param osize The old size of the memory region that ptr points to
 * @param nsize The desired new size of the memory region that ptr points to
 */
void *lua_zephyr_allocator(void *ud, void *ptr, size_t osize, size_t nsize)
{
	void *res = NULL;

	if (nsize == 0) {
		if (ptr != NULL) {
			sys_heap_free(ud, ptr);
		}

	} else {
		res = sys_heap_realloc(ud, ptr, nsize);
		if (res == NULL) {
			printk("Something went wrong with allocation\n");
		}
	}

	return res;
}

/** @brief Lua binding for k_msleep. Expects one integer argument (ms). */
static int k_msleep_wrapper(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	int ms = luaL_checkinteger(L, 1);

	k_msleep(ms);

	return 0;
}

/** @brief Lua binding for printk. Expects one string argument. */
static int printk_wrapper(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	const char *message = luaL_checkstring(L, 1);

	printk("%s\n", message);

	return 0;
}

/** @brief Lua binding for LOG_INF. Expects one string argument. */
static int log_inf_wrapper(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	const char *message = luaL_checkstring(L, 1);

	LOG_INF("[%s]: %s", k_thread_name_get(k_current_get()), message);

	return 0;
}

/** @brief Lua binding for LOG_WRN. Expects one string argument. */
static int log_wrn_wrapper(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	const char *message = luaL_checkstring(L, 1);

	LOG_WRN("[%s]: %s", k_thread_name_get(k_current_get()), message);

	return 0;
}

/** @brief Lua binding for LOG_DBG. Expects one string argument. */
static int log_dbg_wrapper(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	const char *message = luaL_checkstring(L, 1);

	LOG_DBG("[%s]: %s", k_thread_name_get(k_current_get()), message);

	return 0;
}

/** @brief Lua binding for LOG_ERR. Expects one string argument. */
static int log_err_wrapper(lua_State *L)
{
	int n = lua_gettop(L);
	if (n != 1) {
		return luaL_error(L, "expected 1 arguments, got %d", n);
	}

	const char *message = luaL_checkstring(L, 1);

	LOG_ERR("[%s]: %s", k_thread_name_get(k_current_get()), message);

	return 0;
}

static const luaL_Reg zephyr_wrappers[] = {
	{"msleep", k_msleep_wrapper},
	{"printk", printk_wrapper},
	{"log_inf", log_inf_wrapper},
	{"log_wrn", log_wrn_wrapper},
	{"log_dbg", log_dbg_wrapper},
	{"log_err", log_err_wrapper},
	{NULL, NULL} // Sentinel value to mark the end of the array
};

/** @brief Open the `zephyr` Lua library. Registers kernel wrappers and nests zbus/fs. */
int luaopen_zephyr(lua_State *L)
{
	luaL_newlib(L, zephyr_wrappers);

	/* Nest zbus as zephyr.zbus */
	luaopen_zbus(L);
	lua_setfield(L, -2, "zbus");

#ifdef CONFIG_LUA_FS
	/* Nest fs as zephyr.fs */
	luaopen_fs(L);
	lua_setfield(L, -2, "fs");
#endif

	return 1;
}

/** @brief Load base + package libs and preload zephyr and standard Lua libs. */
void luaz_openlibs(lua_State *L)
{
	LUA_REQUIRE(base);
	LUA_REQUIRE(package);

	/* Preload zephyr (includes zbus, fs) and standard Lua libs */
	luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);

	lua_pushcfunction(L, luaopen_zephyr);
	lua_setfield(L, -2, "zephyr");

	lua_pushcfunction(L, luaopen_string);
	lua_setfield(L, -2, "string");
	lua_pushcfunction(L, luaopen_table);
	lua_setfield(L, -2, "table");
	lua_pushcfunction(L, luaopen_math);
	lua_setfield(L, -2, "math");
	lua_pushcfunction(L, luaopen_coroutine);
	lua_setfield(L, -2, "coroutine");
	lua_pushcfunction(L, luaopen_utf8);
	lua_setfield(L, -2, "utf8");
	lua_pushcfunction(L, luaopen_debug);
	lua_setfield(L, -2, "debug");

	lua_pop(L, 1); /* pop preload table */
}

/** @brief POSIX stub — _times is unused but required by the toolchain. */
clock_t _times(struct tms *tms)
{
	errno = ENOSYS;
	return (clock_t)-1;
}

/** @brief POSIX stub — _unlink is unused but required by the toolchain. */
int _unlink(const char *pathname)
{
	return -1;
}

/** @brief POSIX stub — _link is unused but required by the toolchain. */
int _link(const char *oldpath, const char *newpath)
{
	errno = ENOSYS;
	return -1;
}
