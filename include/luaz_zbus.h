/**
 * @file luaz_zbus.h
 * @brief Lua bindings for Zephyr zbus channels and observers.
 *
 * Provides the zbus Lua library opener.  Channels and observers are
 * declared from Lua via zbus.channel_declare() / zbus.observer_declare().
 */

#ifndef _LUAZ_ZBUS_H
#define _LUAZ_ZBUS_H

#include <lua.h>

/**
 * @brief Open the `zbus` Lua library (channel/observer metatables).
 *
 * @param L  Lua state.
 * @return 1 (the library table is on the stack).
 */
int luaopen_zbus(lua_State *L);

#endif /* _LUAZ_ZBUS_H */
