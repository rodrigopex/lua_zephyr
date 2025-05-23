#include "lua_zephyr/utils.h"

#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <time.h>
#include <zephyr/sys/sys_heap.h>
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

static int add_numbers(lua_State *L)
{
	// Get the number of arguments
	int n = lua_gettop(L);
	if (n != 2) {
		return luaL_error(L, "expected 2 arguments, got %d", n);
	}
	// Get the arguments from the stack
	float a = luaL_checknumber(L, 1);
	float b = luaL_checknumber(L, 2);

	// Perform the addition
	float sum = a + b;

	// Push the result onto the stack
	lua_pushnumber(L, sum);

	// Return the number of return values
	return 1;
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

	printk("[lua]: %s\n", message);

	return 0;
}

static const luaL_Reg c_wrappers[] = {
	{"add", add_numbers},
	{"msleep", k_msleep_wrapper},
	{"printk", printk_wrapper},
	{NULL, NULL} // Sentinel value to mark the end of the array
};

static int luaopen_zephyr(lua_State *L)
{
	luaL_newlib(L, c_wrappers);
	return 1;
}

int lua_register_zephyr_api(lua_State *L)
{
	luaL_requiref(L, "zephyr", luaopen_zephyr, 1);
	lua_pop(L, 1);

	return 0;
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
