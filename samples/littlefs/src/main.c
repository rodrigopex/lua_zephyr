/**
 * @file main.c
 * @brief LittleFS sample: mount filesystem and bootstrap Lua scripts.
 *
 * Mounts a LittleFS partition (auto-formatting on first boot), then writes
 * the embedded Lua scripts to the filesystem so the FS-backed Lua thread
 * can load them at runtime.
 */

#include <zephyr/kernel.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
#include <lua_zephyr/lua_fs.h>

#include "hello_fs_lua_script.h"
#include "greet_lua_script.h"

LOG_MODULE_REGISTER(littlefs_sample);

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);

static struct fs_mount_t lua_lfs_mount = {
	.type = FS_LITTLEFS,
	.fs_data = &storage,
	.storage_dev = (void *)FIXED_PARTITION_ID(storage_partition),
	.mnt_point = CONFIG_LUA_FS_MOUNT_POINT,
};

/** @brief Mount LittleFS, auto-format on failure. */
static int mount_fs(void)
{
	int rc = fs_mount(&lua_lfs_mount);

	if (rc < 0) {
		LOG_WRN("fs_mount failed: %d, formatting...", rc);
		rc = fs_mkfs(FS_LITTLEFS, (uintptr_t)FIXED_PARTITION_ID(storage_partition), NULL,
			     0);
		if (rc < 0) {
			LOG_ERR("fs_mkfs failed: %d", rc);
			return rc;
		}
		rc = fs_mount(&lua_lfs_mount);
		if (rc < 0) {
			LOG_ERR("fs_mount after format failed: %d", rc);
			return rc;
		}
	}

	LOG_INF("LittleFS mounted at %s", CONFIG_LUA_FS_MOUNT_POINT);
	return 0;
}

int main(void)
{
	int rc = mount_fs();

	if (rc < 0) {
		printk("Failed to mount filesystem: %d\n", rc);
		return rc;
	}

	printk("Bootstrap: writing scripts to LittleFS\n");

	lua_fs_write_file("/lfs/hello_fs.lua", hello_fs_lua_script, 0);
	lua_fs_write_file("/lfs/greet.lua", greet_lua_script, 0);

	printk("Bootstrap: done\n");

	k_sleep(K_FOREVER);
	return 0;
}
