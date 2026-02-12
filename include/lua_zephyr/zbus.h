/**
 * @file zbus.h
 * @brief Lua bindings for Zephyr zbus channels and observers.
 *
 * Provides macros and functions to expose zbus channels and observers
 * as Lua userdata inside a Lua state.  Used by lua_thread setup hooks
 * to make zbus objects available to Lua scripts.
 */

#ifndef _LUA_ZBUS_H
#define _LUA_ZBUS_H

#include <lua.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

/** @brief Helper to take the address of a symbol. */
#define __REF(_a) &_a

/**
 * @brief Register a zbus channel as a named field in the Lua `zbus` table.
 *
 * Assumes the Lua state variable is named `L`.
 *
 * @param _chan  zbus channel variable (unquoted).
 */
#define LUA_ZBUS_CHAN_DECLARE(_chan) lua_zbus_chan_declare(L, __REF(_chan), #_chan)

/**
 * @brief Register a zbus observer as a named field in the Lua `zbus` table.
 *
 * Assumes the Lua state variable is named `L`.
 *
 * @param _obs  zbus observer variable (unquoted).
 */
#define LUA_ZBUS_OBS_DECLARE(_obs) lua_zbus_obs_declare(L, __REF(_obs), #_obs)

/**
 * @brief Push a zbus channel userdata into the Lua `zbus` global table.
 *
 * @param L          Lua state.
 * @param chan       Pointer to the zbus channel.
 * @param chan_name  Name used as the table key in Lua.
 * @return 1 on success.
 */
int lua_zbus_chan_declare(lua_State *L, const struct zbus_channel *chan, const char *chan_name);

/**
 * @brief Push a zbus observer userdata into the Lua `zbus` global table.
 *
 * @param L         Lua state.
 * @param obs       Pointer to the zbus observer.
 * @param obs_name  Name used as the table key in Lua.
 * @return 1 on success.
 */
int lua_zbus_obs_declare(lua_State *L, const struct zbus_observer *obs, const char *obs_name);

/**
 * @brief Open the `zbus` Lua library (channel/observer metatables).
 *
 * @param L  Lua state.
 * @return 1 (the library table is on the stack).
 */
int luaopen_zbus(lua_State *L);

#endif /* _LUA_ZBUS_H */
