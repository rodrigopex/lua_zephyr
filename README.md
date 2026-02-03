# Overview

This is an initiative to try making Lua a module on Zephyr. There have been several attempts to achieve this in the past, but it remains elusive. This effort aims to make Lua an alternative for implementing application logic in embedded projects that rely on Zephyr RTOS.

# The mission

- Be simple to use: `lua_add_thread("src/some_script.lua")`
- Be flexible (giving the users ways to implement low-level things)
- Has a minimum bindings set (sleeps, logs, and zbus)
- Be as performant as possible

# Overview of the idea

We are assuming a project pattern where Lua scripts can only interact with C code via zbus channels and available kernel APIs. Every Lua file will run in its dedicated thread context and dedicated heap.

![image](https://github.com/user-attachments/assets/c6bc2fc3-6ba3-45a1-98c0-98e4dd28f1bf)

# Lua Table <-> C Struct Mapping

This project implements a "codec" that supports automatic runtime mapping between Lua tables and C structs in zbus messages. As of now, it only supports "flat" -- i.e., non-nested -- structures.

## Example

```c
static const struct lua_zephyr_table_descr version[] = {
 LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_version, major, LUA_CODEC_VALUE_TYPE_INTEGER),
 LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_version, minor, LUA_CODEC_VALUE_TYPE_INTEGER),
 LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_version, patch, LUA_CODEC_VALUE_TYPE_INTEGER),
 LUA_TABLE_FIELD_DESCRIPTOR_PRIM(struct msg_version, hardware_id,
     LUA_CODEC_VALUE_TYPE_STRING),
};
LUA_ZEPHYR_WRAPPER_DESC(version);
```

The snippet above defines a mapping between a Lua table and the C struct `struct msg_version`. The macro `LUA_TABLE_FIELD_DESCRIPTOR_PRIM` is used to define each field in the struct, specifying the field name and its corresponding Lua codec value type.

Then, in Lua:

```lua
local err, msg = zbus.chan_version:read(500)
if msg then
 zephyr.printk("System version: v" .. msg.major .. "." .. msg.minor .. "." .. msg.patch .. "_" .. msg.hardware_id)
end
```

The code above reads a message from the `chan_version` zbus channel and accesses the fields of the `msg_version` struct as properties of a Lua table.
