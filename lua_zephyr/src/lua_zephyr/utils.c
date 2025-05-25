#include "lua.h"
#include "lua_zephyr/utils.h"

#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <time.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/logging/log.h>

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

int luaopen_zephyr(lua_State *L)
{
	luaL_newlib(L, zephyr_wrappers);
	return 1;
}

/* Stub function just to make the compiler happy */
clock_t _times(struct tms *tms)
{
	errno = ENOSYS;
	return (clock_t)-1;
}

/* Stub function just to make the compiler happy */
int _unlink(const char *pathname)
{
	return -1; // fs_unlink(pathname);
}

/* Stub function just to make the compiler happy */
int _link(const char *oldpath, const char *newpath)
{
	errno = ENOSYS;
	return -1;
}
