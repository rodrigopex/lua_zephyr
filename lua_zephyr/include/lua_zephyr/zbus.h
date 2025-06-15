
#ifndef _LUA_ZBUS_H
#define _LUA_ZBUS_H

#include <lua.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

#define __REF(_a) &_a

#define LUA_ZBUS_CHAN_DECLARE(_chan) lua_zbus_chan_declare(L, __REF(_chan), #_chan)

#define LUA_ZBUS_OBS_DECLARE(_obs) lua_zbus_obs_declare(L, __REF(_obs), #_obs)

int lua_zbus_chan_declare(lua_State *L, const struct zbus_channel *chan, const char *chan_name);

int lua_zbus_obs_declare(lua_State *L, const struct zbus_observer *obs, const char *obs_name);

int luaopen_zbus(lua_State *L);

#endif /* _LUA_ZBUS_H */
