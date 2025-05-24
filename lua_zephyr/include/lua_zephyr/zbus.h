
#ifndef _LUA_ZBUS_H
#define _LUA_ZBUS_H

#include <lua.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

#define REF(_a) &_a

#define LUA_ZBUS_CHAN_DECLARE(_chan) lua_zbus_chan_declare(L, REF(_chan), #_chan)

#define LUA_ZBUS_OBS_DECLARE(_obs) lua_zbus_obs_declare(L, REF(_obs), #_obs)

#define LUA_TABLE_SET(_k_string, _v_lua_type_name, _v)                                             \
	lua_pushstring(L, _k_string);                                                              \
	lua_push##_v_lua_type_name(L, _v);                                                         \
	lua_settable(L, -3);

#define LUA_TABLE_GET(_k_string, _var_lua_type_name)                                               \
	({                                                                                         \
		lua_getfield(L, 2, _k_string);                                                     \
		lua_to##_var_lua_type_name(L, -1);                                                 \
	})

int lua_zbus_chan_declare(lua_State *L, const struct zbus_channel *chan, const char *chan_name);

int lua_zbus_obs_declare(lua_State *L, const struct zbus_observer *obs, const char *obs_name);

int luaopen_zbus(lua_State *L);

#endif /* _LUA_ZBUS_H */
