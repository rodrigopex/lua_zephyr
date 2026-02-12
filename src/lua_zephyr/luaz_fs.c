/**
 * @file luaz_fs.c
 * @brief Lua filesystem support: script loading and Lua library.
 *
 * Provides C helpers for loading/writing Lua scripts from any mounted
 * filesystem, and exposes an `fs` Lua library with dofile, loadfile, and
 * list functions.  Also replaces the standard Lua globals dofile and
 * loadfile with FS-backed versions.
 *
 * The filesystem mount is the application's responsibility — this library
 * only uses the generic Zephyr FS API (<zephyr/fs/fs.h>).
 */

#ifdef CONFIG_LUA_FS

#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lua_zephyr/luaz_fs.h>
#include <lua_zephyr/luaz_utils.h>
#include <zephyr/kernel.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(lua_zephyr, CONFIG_LUA_ZEPHYR_LOG_LEVEL);

#define MOUNT_POINT CONFIG_LUA_FS_MOUNT_POINT
#define MAX_PATH    64

/**
 * @brief Build an absolute path from a user-provided path.
 *
 * If @p path starts with '/', it is used as-is.  Otherwise the mount point
 * is prepended (e.g. "greet.lua" -> "/lfs/greet.lua").
 *
 * @param buf   Output buffer (at least MAX_PATH bytes).
 * @param path  User-provided path.
 * @return 0 on success, -ENAMETOOLONG if result exceeds MAX_PATH.
 */
static int build_path(char *buf, const char *path)
{
	if (path[0] == '/') {
		if (strlen(path) >= MAX_PATH) {
			return -ENAMETOOLONG;
		}
		strcpy(buf, path);
	} else {
		int ret = snprintf(buf, MAX_PATH, "%s/%s", MOUNT_POINT, path);

		if (ret < 0 || ret >= MAX_PATH) {
			return -ENAMETOOLONG;
		}
	}
	return 0;
}

/**
 * @brief Read a file into a buffer allocated from the Lua state's heap.
 *
 * @param L         Lua state (used for allocation).
 * @param fullpath  Absolute file path.
 * @param out_buf   Receives pointer to allocated buffer (caller must free).
 * @param out_len   Receives file size.
 * @return 0 on success, negative errno on failure.
 */
static int read_file_into_buf(lua_State *L, const char *fullpath, char **out_buf, size_t *out_len)
{
	struct fs_dirent entry;
	int rc = fs_stat(fullpath, &entry);

	if (rc < 0) {
		LOG_ERR("fs_stat(%s) failed: %d", fullpath, rc);
		return rc;
	}

	if (entry.type != FS_DIR_ENTRY_FILE) {
		return -EISDIR;
	}

	if (entry.size > CONFIG_LUA_FS_MAX_FILE_SIZE) {
		LOG_ERR("File %s too large: %zu > %d", fullpath, entry.size,
			CONFIG_LUA_FS_MAX_FILE_SIZE);
		return -EFBIG;
	}

	/* Allocate from the Lua state's heap */
	void *ud;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	char *buf = allocf(ud, NULL, 0, entry.size + 1);

	if (buf == NULL) {
		LOG_ERR("Failed to allocate %zu bytes for %s", entry.size + 1, fullpath);
		return -ENOMEM;
	}

	struct fs_file_t file;

	fs_file_t_init(&file);

	rc = fs_open(&file, fullpath, FS_O_READ);
	if (rc < 0) {
		LOG_ERR("fs_open(%s) failed: %d", fullpath, rc);
		allocf(ud, buf, entry.size + 1, 0);
		return rc;
	}

	ssize_t bytes_read = fs_read(&file, buf, entry.size);

	fs_close(&file);

	if (bytes_read < 0) {
		LOG_ERR("fs_read(%s) failed: %zd", fullpath, bytes_read);
		allocf(ud, buf, entry.size + 1, 0);
		return (int)bytes_read;
	}

	buf[bytes_read] = '\0';
	*out_buf = buf;
	*out_len = (size_t)bytes_read;
	return 0;
}

int lua_fs_dofile(lua_State *L, const char *path)
{
	char fullpath[MAX_PATH];
	int rc = build_path(fullpath, path);

	if (rc < 0) {
		return rc;
	}

	char *buf = NULL;
	size_t len = 0;

	rc = read_file_into_buf(L, fullpath, &buf, &len);
	if (rc < 0) {
		lua_pushfstring(L, "cannot open %s: error %d", fullpath, rc);
		return rc;
	}

	void *ud;
	lua_Alloc allocf = lua_getallocf(L, &ud);

	rc = luaL_loadbuffer(L, buf, len, fullpath);
	if (rc != LUA_OK) {
		allocf(ud, buf, len + 1, 0);
		return rc;
	}

	allocf(ud, buf, len + 1, 0);

	rc = lua_pcall(L, 0, LUA_MULTRET, 0);
	return rc;
}

int lua_fs_loadfile(lua_State *L, const char *path)
{
	char fullpath[MAX_PATH];
	int rc = build_path(fullpath, path);

	if (rc < 0) {
		return rc;
	}

	char *buf = NULL;
	size_t len = 0;

	rc = read_file_into_buf(L, fullpath, &buf, &len);
	if (rc < 0) {
		lua_pushfstring(L, "cannot open %s: error %d", fullpath, rc);
		return rc;
	}

	void *ud;
	lua_Alloc allocf = lua_getallocf(L, &ud);

	rc = luaL_loadbuffer(L, buf, len, fullpath);
	allocf(ud, buf, len + 1, 0);

	return rc;
}

int lua_fs_write_file(const char *path, const char *data, size_t len)
{
	struct fs_file_t file;

	fs_file_t_init(&file);

	if (len == 0) {
		len = strlen(data);
	}

	int rc = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE);

	if (rc < 0) {
		LOG_ERR("fs_open(%s) for write failed: %d", path, rc);
		return rc;
	}

	/* Truncate to zero before writing */
	rc = fs_truncate(&file, 0);
	if (rc < 0) {
		LOG_ERR("fs_truncate(%s) failed: %d", path, rc);
		fs_close(&file);
		return rc;
	}

	ssize_t written = fs_write(&file, data, len);

	fs_close(&file);

	if (written < 0) {
		LOG_ERR("fs_write(%s) failed: %zd", path, written);
		return (int)written;
	}

	if ((size_t)written != len) {
		LOG_ERR("fs_write(%s): short write %zd/%zu", path, written, len);
		return -ENOSPC;
	}

	return 0;
}

/* --- Lua library functions --- */

/** @brief Lua function: fs.dofile(path) — load and execute a script from FS. */
static int l_fs_dofile(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	int top_before = lua_gettop(L) - 1; /* exclude the path argument */

	int rc = lua_fs_dofile(L, path);

	if (rc != LUA_OK && rc < 0) {
		return luaL_error(L, "%s", lua_tostring(L, -1));
	}
	if (rc != LUA_OK) {
		return lua_error(L);
	}

	return lua_gettop(L) - top_before;
}

/** @brief Lua function: fs.loadfile(path) — load a script without executing. */
static int l_fs_loadfile(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);
	int rc = lua_fs_loadfile(L, path);

	if (rc != LUA_OK && rc < 0) {
		return luaL_error(L, "%s", lua_tostring(L, -1));
	}
	if (rc != LUA_OK) {
		lua_pushnil(L);
		lua_insert(L, -2);
		return 2;
	}

	return 1;
}

/** @brief Lua function: fs.list([path]) — list files in directory. */
static int l_fs_list(lua_State *L)
{
	const char *path = luaL_optstring(L, 1, MOUNT_POINT);
	char fullpath[MAX_PATH];

	if (path[0] != '/') {
		snprintf(fullpath, sizeof(fullpath), "%s/%s", MOUNT_POINT, path);
		path = fullpath;
	}

	struct fs_dir_t dir;

	fs_dir_t_init(&dir);

	int rc = fs_opendir(&dir, path);

	if (rc < 0) {
		return luaL_error(L, "cannot open directory %s: error %d", path, rc);
	}

	lua_newtable(L);
	int idx = 1;

	while (true) {
		struct fs_dirent entry;

		rc = fs_readdir(&dir, &entry);
		if (rc < 0 || entry.name[0] == '\0') {
			break;
		}

		lua_newtable(L);
		lua_pushstring(L, entry.name);
		lua_setfield(L, -2, "name");
		lua_pushinteger(L, entry.size);
		lua_setfield(L, -2, "size");
		lua_pushstring(L, entry.type == FS_DIR_ENTRY_DIR ? "dir" : "file");
		lua_setfield(L, -2, "type");

		lua_rawseti(L, -2, idx++);
	}

	fs_closedir(&dir);
	return 1;
}

static const luaL_Reg fs_lib[] = {
	{"dofile", l_fs_dofile},
	{"loadfile", l_fs_loadfile},
	{"list", l_fs_list},
	{NULL, NULL},
};

int luaopen_fs(lua_State *L)
{
	luaL_newlib(L, fs_lib);

	/* Replace global dofile and loadfile with FS-backed versions */
	lua_pushcfunction(L, l_fs_dofile);
	lua_setglobal(L, "dofile");

	lua_pushcfunction(L, l_fs_loadfile);
	lua_setglobal(L, "loadfile");

	return 1;
}

#endif /* CONFIG_LUA_FS */
