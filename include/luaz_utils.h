/**
 * @file luaz_utils.h
 * @brief Core Lua-Zephyr utilities: allocator, kernel bindings, and helper macros.
 *
 * Provides the custom sys_heap-backed allocator for Lua states and convenience
 * macros for library loading and table manipulation.
 */

#ifndef _LUAZ_UTILS_H
#define _LUAZ_UTILS_H

#include <lua.h>
#include <zephyr/kernel.h>

/* clang-format off */

/**
 * @brief Load a Lua standard or custom library into the current state.
 *
 * Calls luaL_requiref to register the library globally, then pops the
 * module table from the stack.  Assumes the Lua state variable is named `L`.
 *
 * @param _lib  Library name (unquoted); must have a matching luaopen_##_lib.
 */
#define LUA_REQUIRE(_lib)                             \
	do {                                              \
		luaL_requiref(L, #_lib, luaopen_##_lib, 1);   \
		lua_pop(L, 1);                                \
	} while (0)

/**
 * @brief Push a key/value pair onto the table at the top of the Lua stack.
 *
 * @param _k_string          String key.
 * @param _v_lua_type_name   Lua push suffix (e.g. `integer`, `number`, `string`).
 * @param _v                 Value to push.
 */
#define LUA_TABLE_SET(_k_string, _v_lua_type_name, _v)   \
	lua_pushstring(L, _k_string);                        \
	lua_push##_v_lua_type_name(L, _v);                   \
	lua_settable(L, -3);

/**
 * @brief Read a field from the table at stack index 2 and convert it.
 *
 * Returns the converted value via a GCC statement-expression.
 *
 * @param _k_string            String key to look up.
 * @param _var_lua_type_name   Lua conversion suffix (e.g. `integer`, `number`).
 */
#define LUA_TABLE_GET(_k_string, _var_lua_type_name)   \
	({                                                 \
		lua_getfield(L, 2, _k_string);                 \
		lua_to##_var_lua_type_name(L, -1);             \
	})
/* clang-format on */

/**
 * @brief Custom Lua allocator backed by a Zephyr sys_heap.
 *
 * Conforms to the lua_Alloc signature.  Passed as the allocator function
 * to lua_newstate, with the sys_heap pointer as @p ud.
 *
 * @param ud     Pointer to the struct sys_heap used for allocation.
 * @param ptr    Pointer to the existing block (NULL for new allocations).
 * @param osize  Original block size.
 * @param nsize  Requested new size (0 to free).
 * @return Pointer to the (re)allocated block, or NULL on free / failure.
 */
void *lua_zephyr_allocator(void *ud, void *ptr, size_t osize, size_t nsize);

/**
 * @brief Open the `zephyr` Lua library (msleep, printk, log_*).
 *
 * @param L  Lua state.
 * @return 1 (the library table is on the stack).
 */
int luaopen_zephyr(lua_State *L);

#endif /* _LUAZ_UTILS_H */
