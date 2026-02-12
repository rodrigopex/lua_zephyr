/**
 * @file luaz_fs.h
 * @brief Lua filesystem support: script loading and Lua library.
 *
 * Provides functions to load and execute Lua scripts from any mounted
 * filesystem, write files for bootstrap, and a Lua library (fs.dofile,
 * fs.loadfile, fs.list).  The application is responsible for mounting
 * the filesystem before using these functions.
 */

#ifndef _LUAZ_FS_H
#define _LUAZ_FS_H

#include <lua.h>

/**
 * @brief Load and execute a Lua script from the filesystem.
 *
 * Reads the file at @p path, loads it via luaL_loadbuffer, and executes it
 * with lua_pcall.  If @p path does not start with '/', the configured mount
 * point is prepended.
 *
 * @param L     Lua state.
 * @param path  File path (absolute or relative to mount point).
 * @return 0 on success, negative errno or LUA_ERRRUN on failure.
 */
int lua_fs_dofile(lua_State *L, const char *path);

/**
 * @brief Load a Lua script from the filesystem without executing it.
 *
 * Reads the file at @p path, loads it via luaL_loadbuffer, and pushes the
 * resulting function onto the Lua stack.
 *
 * @param L     Lua state.
 * @param path  File path (absolute or relative to mount point).
 * @return 0 on success, negative errno or Lua error code on failure.
 */
int lua_fs_loadfile(lua_State *L, const char *path);

/**
 * @brief Write data to a file on the filesystem.
 *
 * Creates or overwrites the file at @p path.  If @p len is 0, strlen(data)
 * is used.
 *
 * @param path  Absolute file path.
 * @param data  Data to write.
 * @param len   Data length (0 = use strlen).
 * @return 0 on success, negative errno on failure.
 */
int lua_fs_write_file(const char *path, const char *data, size_t len);

/**
 * @brief Open the `fs` Lua library.
 *
 * Registers fs.dofile, fs.loadfile, fs.list and replaces the global
 * dofile and loadfile with FS-backed versions.
 *
 * @param L  Lua state.
 * @return 1 (the library table is on the stack).
 */
int luaopen_fs(lua_State *L);

#endif /* _LUAZ_FS_H */
