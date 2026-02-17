---@meta

--- zbus channel userdata (registered via LUA_REQUIRE_ZBUS_CHAN).
---@class zbus_channel
local zbus_channel = {}

--- Publish a message table to this channel.
---@param data table # Message table matching the channel descriptor.
---@param timeout_ms integer # Timeout in milliseconds.
---@return integer err # 0 on success, negative errno on failure.
function zbus_channel:pub(data, timeout_ms) end

--- Read the current message from this channel.
---@param timeout_ms integer # Timeout in milliseconds.
---@return integer err # 0 on success, negative errno on failure.
---@return table|nil data # Message table, or nil on error.
function zbus_channel:read(timeout_ms) end

--- zbus observer userdata (registered via LUA_REQUIRE_ZBUS_OBS).
---@class zbus_observer
local zbus_observer = {}

--- Wait for a message on any subscribed channel.
---@param timeout_ms integer # Timeout in milliseconds.
---@return integer err # 0 on success, negative errno on failure.
---@return zbus_channel|nil channel # Source channel, or nil on error.
---@return table|nil data # Message table, or nil on error.
function zbus_observer:wait_msg(timeout_ms) end

--- zbus pub/sub namespace (requires CONFIG_LUA_LIB_ZBUS).
---@class zbus
local zbus = {}

--- Declare (look up) a zbus channel by name.
--- Mirrors Zephyr's ZBUS_CHAN_DECLARE — makes the channel available to Lua.
---@param name string # Channel name (e.g. "chan_acc_data").
---@return zbus_channel channel # Channel userdata.
function zbus.channel_declare(name) end

--- Declare (look up) a zbus observer by name.
--- Mirrors Zephyr's ZBUS_OBS_DECLARE — makes the observer available to Lua.
---@param name string # Observer name (e.g. "msub_acc_consumed").
---@return zbus_observer observer # Observer userdata.
function zbus.observer_declare(name) end

--- Filesystem API (requires CONFIG_LUA_FS).
---@class fs
local fs = {}

--- Load and execute a Lua script from the filesystem.
---@param path string # File path (relative to mount point or absolute).
---@return any ... # Values returned by the script.
function fs.dofile(path) end

--- Load a Lua script from the filesystem without executing it.
---@param path string # File path (relative to mount point or absolute).
---@return function|nil chunk # Compiled chunk, or nil on error.
---@return string|nil err # Error message on failure.
function fs.loadfile(path) end

--- List files in a directory.
---@param path? string # Directory path (defaults to mount point).
---@return table entries # Array of {name: string, size: integer, type: string}.
function fs.list(path) end

--- Zephyr kernel API bindings.
--- Access via: local zephyr = require("zephyr")
---@class zephyr
---@field zbus zbus # zbus pub/sub namespace (requires CONFIG_LUA_LIB_ZBUS).
---@field fs fs # Filesystem API (requires CONFIG_LUA_FS).
local zephyr = {}

--- Sleep for the specified number of milliseconds.
---@param ms integer # Duration in milliseconds.
function zephyr.msleep(ms) end

--- Print a message via Zephyr printk.
---@param message string # Message to print.
function zephyr.printk(message) end

--- Log a message at INFO level.
---@param message string # Message to log.
function zephyr.log_inf(message) end

--- Log a message at WARNING level.
---@param message string # Message to log.
function zephyr.log_wrn(message) end

--- Log a message at DEBUG level.
---@param message string # Message to log.
function zephyr.log_dbg(message) end

--- Log a message at ERROR level.
---@param message string # Message to log.
function zephyr.log_err(message) end

return zephyr
