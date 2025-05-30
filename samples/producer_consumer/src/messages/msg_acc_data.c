#include "channels.h"

#include <lauxlib.h>

// Metatable name for our userdata
#define MSG_ACC_DATA_METATABLE "msg_acc_data_mt"

/*** Metatable methods ***/

// __index metamethod: Handles reading members (e.g., acc.x)
static int msg_acc_data_index(lua_State *L)
{
	// Get the userdata object
	struct msg_acc_data *acc =
		(struct msg_acc_data *)luaL_checkudata(L, 1, MSG_ACC_DATA_METATABLE);
	// Get the key (member name)
	const char *key = luaL_checkstring(L, 2);

	if (strcmp(key, "x") == 0) {
		lua_pushinteger(L, acc->x);
	} else if (strcmp(key, "y") == 0) {
		lua_pushinteger(L, acc->y);
	} else if (strcmp(key, "z") == 0) {
		lua_pushinteger(L, acc->z);
	} else {
		// Return nil for unknown members
		lua_pushnil(L);
	}
	return 1; // Number of return values
}

// __newindex metamethod: Handles writing members (e.g., acc.x = 10)
static int msg_acc_data_newindex(lua_State *L)
{
	// Get the userdata object
	struct msg_acc_data *acc =
		(struct msg_acc_data *)luaL_checkudata(L, 1, MSG_ACC_DATA_METATABLE);
	// Get the key (member name)
	const char *key = luaL_checkstring(L, 2);
	// Get the new value
	lua_Integer value = luaL_checkinteger(L, 3);

	if (strcmp(key, "x") == 0) {
		acc->x = (int)value;
	} else if (strcmp(key, "y") == 0) {
		acc->y = (int)value;
	} else if (strcmp(key, "z") == 0) {
		acc->z = (int)value;
	} else {
		// Optionally, you could raise an error for attempting to set unknown members
		luaL_error(L, "attempt to set unknown member '%s' on msg_acc_data", key);
	}
	return 0; // No return values
}

// __gc metamethod: Handles garbage collection (frees allocated memory)
static int msg_acc_data_gc(lua_State *L)
{
	struct msg_acc_data *acc =
		(struct msg_acc_data *)luaL_checkudata(L, 1, MSG_ACC_DATA_METATABLE);
	if (acc) {
		// In this example, we allocated with lua_newuserdata, so Lua handles the memory.
		// If you used malloc, you would free it here:
		// free(acc);
		printk(" ~~> msg_acc_data %p userdata garbage collected!\n", acc);
	}
	return 0;
}

static int msg_acc_data_tostring(lua_State *L)
{
	struct msg_acc_data *acc =
		(struct msg_acc_data *)luaL_checkudata(L, 1, MSG_ACC_DATA_METATABLE);

	if (acc == NULL) {
		return luaL_error(L, "invalid msg_acc_data");
	}

	lua_pushfstring(L, "msg.AccData { x=%d, y=%d, z=%d } ", acc->x, acc->y, acc->z);
	return 1;
}

/*** Userdata creation function ***/

// Function to create a new msg_acc_data userdata from Lua
static int new_msg_acc_data(lua_State *L)
{
	// Allocate userdata for msg_acc_data
	struct msg_acc_data *acc =
		(struct msg_acc_data *)lua_newuserdata(L, sizeof(struct msg_acc_data));

	// Initialize members
	acc->x = 0;
	acc->y = 0;
	acc->z = 0;

	// Set the metatable for the new userdata
	luaL_getmetatable(L, MSG_ACC_DATA_METATABLE);
	lua_setmetatable(L, -2); // Set metatable for the userdata at -2

	return 1; // Return the new userdata
}

/*** Library registration ***/

// Array of functions to be registered in Lua
static const struct luaL_Reg msg_acc_data_methods[] = {
	{"new", new_msg_acc_data}, {NULL, NULL} // Sentinel
};

// Array of metamethods for the metatable
static const struct luaL_Reg msg_acc_data_metamethods[] = {
	{"__index", msg_acc_data_index},
	{"__newindex", msg_acc_data_newindex},
	{"__gc", msg_acc_data_gc},
	{"__tostring", msg_acc_data_tostring},
	{NULL, NULL} // Sentinel
};

// Lua open function for the module
int luaopen_msg_acc_data(lua_State *L)
{
	// Create the metatable if it doesn't exist
	luaL_newmetatable(L, MSG_ACC_DATA_METATABLE);

	// Register metamethods onto the metatable
	luaL_setfuncs(L, msg_acc_data_metamethods, 0);

	// Push the metatable to be accessible via rawget for __gc to work correctly
	// (though lua_newuserdata automatically handles this when you set the metatable)
	lua_pop(L, 1); // Pop the metatable from the stack (it's stored in the registry)

	// Create a table for the module (e.g., `msg_acc_data.new()`)
	luaL_newlib(L, msg_acc_data_methods);

	return 1; // Return the module table
}
