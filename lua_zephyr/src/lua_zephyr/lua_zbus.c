/**
 * @file lua_zbus.c
 * @brief Lua bindings for Zephyr zbus: publish, read, wait, and serialization.
 *
 * Implements channel and observer userdata types with metatables so Lua scripts
 * can call :pub(), :read(), and :wait_msg().  Weak conversion hooks delegate to
 * the descriptor system in lua_msg_descr when channel user_data is set.
 */

#include <lauxlib.h>
#include <lualib.h>
#include <sys/times.h>
#include <zephyr/zbus/zbus.h>
#include <lua_zephyr/utils.h>
#include <lua_zephyr/zbus.h>
#include <lua_zephyr/lua_msg_descr.h>
#include <zephyr/kernel.h>

/** @brief Metatable name for zbus channel userdata. */
#define ZBUS_CHAN_METATABLE "zbus.channel.mt"
/** @brief Metatable name for zbus observer userdata. */
#define ZBUS_OBS_METATABLE  "zbus.observer.mt"

/** @brief Maximum message buffer size for sub_wait_msg. */
#define ZBUS_MSG_CAPACITY 512

/** @brief Validate and return the zbus channel userdata at stack index @p idx. */
static const struct zbus_channel **check_zbus_channel(lua_State *L, int idx)
{
	void *ud = luaL_checkudata(L, idx, ZBUS_CHAN_METATABLE);
	luaL_argcheck(L, ud != NULL, idx, "`zbus channel' expected");
	return ud;
}

/**
 * @brief Convert a C message struct to a Lua table (weak default).
 *
 * Uses the descriptor stored in zbus_chan_user_data if available.
 * Applications may provide a strong override for custom serialization.
 *
 * @param L        Lua state.
 * @param chan     The zbus channel the message belongs to.
 * @param message  Pointer to the raw message buffer.
 * @return 1 (table or nil pushed onto the Lua stack).
 */
int __weak msg_struct_to_lua_table(lua_State *L, const struct zbus_channel *chan, void *message)
{
	const struct lua_msg_descr *descr = zbus_chan_user_data(chan);

	if (descr != NULL) {
		lua_msg_descr_to_table(L, descr->fields, descr->field_count, message);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/**
 * @brief Convert a Lua table to a C message struct (weak default).
 *
 * Uses the descriptor stored in zbus_chan_user_data if available.
 * Applications may provide a strong override for custom deserialization.
 *
 * @param L        Lua state.
 * @param chan     The zbus channel the message belongs to.
 * @param message  Pointer to the output message buffer.
 * @return Message size on success, 0 if no descriptor is available.
 */
size_t __weak lua_table_to_msg_struct(lua_State *L, const struct zbus_channel *chan, void *message)
{
	const struct lua_msg_descr *descr = zbus_chan_user_data(chan);

	if (descr != NULL) {
		lua_msg_descr_from_table(L, descr->fields, descr->field_count, message, 2);
		return descr->msg_size;
	}
	return 0;
}

/** @brief Lua method: channel:pub(table, timeout_ms) -> err. */
static int chan_pub(lua_State *L)
{
	int err = -EINVAL;

	int n = lua_gettop(L);
	if (n != 3) {
		return luaL_error(L, "expected 3 arguments, got %d", n);
	}

	const struct zbus_channel **chan = check_zbus_channel(L, 1);

	luaL_checktype(L, 2, LUA_TTABLE);

	int timeout_ms = luaL_checkinteger(L, 3);

	void *msg = k_malloc(zbus_chan_msg_size(*chan));

	if (msg == NULL) {
		lua_pushinteger(L, -ENOMEM);
		return 1;
	}

	size_t s = lua_table_to_msg_struct(L, *chan, msg);

	if (s) {
		err = zbus_chan_pub(*chan, msg, K_MSEC(timeout_ms));
	}

	k_free(msg);

	lua_pushinteger(L, err);

	return 1;
}

/** @brief Lua method: channel:read(timeout_ms) -> err, table. */
static int chan_read(lua_State *L)
{
	int err = -EINVAL;

	int n = lua_gettop(L);
	if (n != 2) {
		return luaL_error(L, "expected 2 arguments, got %d", n);
	}

	const struct zbus_channel **chan = check_zbus_channel(L, 1);
	int timeout_ms = luaL_checkinteger(L, 2);

	void *msg = k_malloc(zbus_chan_msg_size(*chan));

	if (msg == NULL) {
		lua_pushinteger(L, -ENOMEM);
		lua_pushnil(L);
		return 2;
	}

	err = zbus_chan_read(*chan, msg, K_MSEC(timeout_ms));

	lua_pushinteger(L, err);

	msg_struct_to_lua_table(L, *chan, msg);

	k_free(msg);

	return 2;
}
/** @brief Lua metamethod __eq: compare two channel userdata by pointer. */
static int chan_equals(lua_State *L)
{
	const struct zbus_channel **chan1 = check_zbus_channel(L, 1);
	if (chan1 == NULL) {
		return luaL_error(L, "invalid zbus channel");
	}
	const struct zbus_channel **chan2 = check_zbus_channel(L, 2);
	if (chan2 == NULL) {
		return luaL_error(L, "invalid zbus channel");
	}

	lua_pushboolean(L, *chan1 == *chan2);

	return 1;
}

/** @brief Lua metamethod __tostring: return a human-readable channel string. */
static int chan_tostring(lua_State *L)
{
	const struct zbus_channel **chan = check_zbus_channel(L, 1);

	if (chan == NULL) {
		return luaL_error(L, "invalid zbus channel");
	}

	lua_pushfstring(L, "zbus_channel { ref=%p } ", *chan);
	return 1;
}

static const struct luaL_Reg zbus_chan_metamethods[] = {{"pub", chan_pub},
							{"read", chan_read},
							{"__tostring", chan_tostring},
							{"__eq", chan_equals},
							{NULL, NULL}};

/** @brief Validate and return the zbus observer userdata at stack index @p idx. */
static const struct zbus_observer **check_zbus_observer(lua_State *L, int idx)
{
	void *ud = luaL_checkudata(L, idx, ZBUS_OBS_METATABLE);
	luaL_argcheck(L, ud != NULL, idx, "`zbus observer' expected");
	return ud;
}

/** @brief Lua method: observer:wait_msg(timeout_ms) -> err, channel, table. */
static int sub_wait_msg(lua_State *L)
{
	const struct zbus_channel *chan;

	const struct zbus_observer **obs = check_zbus_observer(L, 1);
	int timeout_ms = luaL_checkinteger(L, 2);

	void *msg = k_malloc(ZBUS_MSG_CAPACITY);

	if (msg == NULL) {
		lua_pushinteger(L, -ENOMEM);
		lua_pushnil(L);
		lua_pushnil(L);
		return 3;
	}

	int err = zbus_sub_wait_msg(*obs, &chan, msg, K_MSEC(timeout_ms));

	lua_pushinteger(L, err);

	if (err) {
		lua_pushnil(L);
		lua_pushnil(L);
	} else {
		const struct zbus_channel **lua_chan =
			lua_newuserdata(L, sizeof(const struct zbus_channel *));
		*lua_chan = chan;
		luaL_getmetatable(L, ZBUS_CHAN_METATABLE);
		lua_setmetatable(L, -2);

		msg_struct_to_lua_table(L, chan, msg);
	}

	k_free(msg);

	return 3;
}

static const struct luaL_Reg zbus_obs_metamethods[] = {{"wait_msg", sub_wait_msg}, {NULL, NULL}};

static const luaL_Reg zbus[] = {{NULL, NULL}};

/** @brief Register a zbus channel as a field in the global `zbus` Lua table. */
int lua_zbus_chan_declare(lua_State *L, const struct zbus_channel *chan, const char *chan_name)
{
	lua_getglobal(L, "zbus");
	if (!lua_istable(L, -1)) {
		return luaL_error(L, "`zbus' is not initialized");
	}

	const struct zbus_channel **chan_ud =
		lua_newuserdata(L, sizeof(const struct zbus_channel *));
	*chan_ud = chan;
	luaL_getmetatable(L, ZBUS_CHAN_METATABLE);
	lua_setmetatable(L, -2);

	lua_setfield(L, -2, chan_name);

	return 1;
}

/** @brief Register a zbus observer as a field in the global `zbus` Lua table. */
int lua_zbus_obs_declare(lua_State *L, const struct zbus_observer *obs, const char *obs_name)
{
	lua_getglobal(L, "zbus");
	if (!lua_istable(L, -1)) {
		return luaL_error(L, "`zbus' is not initialized");
	}

	const struct zbus_observer **obs_ud =
		lua_newuserdata(L, sizeof(const struct zbus_observer *));
	*obs_ud = obs;
	luaL_getmetatable(L, ZBUS_OBS_METATABLE);
	lua_setmetatable(L, -2);

	lua_setfield(L, -2, obs_name);

	return 1;
}

/** @brief Open the `zbus` Lua library. Creates channel and observer metatables. */
int luaopen_zbus(lua_State *L)
{
	luaL_newmetatable(L, ZBUS_CHAN_METATABLE);
	/* Duplicate the metatable for setting __index */
	lua_pushvalue(L, -1);
	/* Set metatable.__index = metatable */
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, zbus_chan_metamethods, 0);
	lua_pop(L, 1);

	luaL_newmetatable(L, ZBUS_OBS_METATABLE);
	/* Duplicate the metatable for setting __index */
	lua_pushvalue(L, -1);
	/* Set metatable.__index = metatable */
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, zbus_obs_metamethods, 0);
	lua_pop(L, 1);

	luaL_newlib(L, zbus);

	return 1;
}
