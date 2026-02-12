# Lua for Zephyr RTOS

> **Alpha** — API may change. Feedback welcome!

A [Zephyr RTOS](https://zephyrproject.org/) module that integrates
[Lua 5.5.0](https://www.lua.org/) as a first-class scripting engine for
embedded systems.

Lua scripts run in **dedicated threads** with **isolated heaps**; inter-thread
communication happens through **zbus channels**, keeping scripting sandboxed
and deterministic.

## Features

- **Embedded scripts** — Lua source compiled into the firmware image as C strings
- **Bytecode precompilation** — optional build-time compilation to bytecode; strip the parser to save ~15-20 KB of flash
- **Filesystem loading** — run scripts from LittleFS (or any Zephyr-supported FS) at runtime
- **zbus bindings** — publish, read, and subscribe to zbus channels directly from Lua
- **Message descriptors** — automatic Lua table ↔ C struct conversion (manual or nanopb-generated)
- **Interactive REPL** — Lua shell over the Zephyr console
- **Kernel API bindings** — `msleep`, `printk`, structured logging (`log_inf`, `log_wrn`, `log_dbg`, `log_err`)

## Quick Start

### Prerequisites

- A [Zephyr workspace](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) (west, SDK, toolchain)
- [`just`](https://github.com/casey/just) command runner (optional but recommended)

### Add the module

Clone (or add as a west module) next to your Zephyr workspace, then point your
application's `CMakeLists.txt` at it:

```cmake
cmake_minimum_required(VERSION 3.20.0)
set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_app)

# Embed a Lua script as a C string header
add_lua_file("src/helper.lua")

# Create a dedicated Lua thread that runs a script
add_lua_thread("src/my_script.lua")

target_sources(app PRIVATE src/main.c)
```

### Minimal Lua script

```lua
-- src/my_script.lua
local z = zephyr

z.printk("Hello from Lua!")
z.msleep(1000)
z.printk("Done.")
```

### Setup hook (main.c)

Each `add_lua_thread("src/my_script.lua")` call expects a setup hook named
`my_script_lua_setup` where you register the libraries the script needs:

```c
#include <lua.h>
#include <lua_zephyr/utils.h>

int my_script_lua_setup(lua_State *L)
{
        LUA_REQUIRE(zephyr);
        return 0;
}

int main(void)
{
        k_sleep(K_FOREVER);
        return 0;
}
```

### Enable Lua in prj.conf

```
CONFIG_LUA=y
```

### Build & run

```sh
just build          # west build -b qemu_cortex_m3 app
just run            # west build -t run
```

## Samples

| Sample | Description | Key Features |
|--------|-------------|--------------|
| `hello_world` | Basic Lua thread + embedded script | `add_lua_thread`, `add_lua_file`, REPL, logging |
| `hello_world_bytecode` | Bytecode-only variant | `add_lua_bytecode_thread`, parser stripped |
| `producer_consumer` | zbus pub/sub between Lua and C | nanopb descriptors, nested structs, bytecode |
| `littlefs` | Scripts loaded from LittleFS at runtime | `add_lua_fs_thread`, `add_lua_fs_file`, `fs.dofile` |
| `heavy` | Stress test with dynamic code generation | Heap usage tracking, recursion, string ops |

Run a sample:

```sh
# Point the build at a sample directory
just build project_dir=samples/hello_world
just run
```

Run the full test suite:

```sh
just test           # west twister -p mps2/an385 -T samples
```

## CMake API

All functions are provided by `lua.cmake` (auto-included when the module is loaded).

| Function | Description |
|----------|-------------|
| `add_lua_thread(path)` | Generate a Zephyr thread that runs an embedded Lua source script |
| `add_lua_bytecode_thread(path)` | Same, but precompile to bytecode at build time (`CONFIG_LUA_PRECOMPILE`) |
| `add_lua_fs_thread(fs_path)` | Generate a thread that loads its script from the filesystem at runtime (`CONFIG_LUA_FS`) |
| `add_lua_file(path)` | Embed a `.lua` file as a C `const char[]` header |
| `add_lua_bytecode_file(path)` | Embed precompiled bytecode as a C `uint8_t[]` header |
| `add_lua_fs_file(src [name])` | Register a Lua file for embedding and writing to the filesystem at boot |

## Lua API

### `zephyr` library

Loaded with `LUA_REQUIRE(zephyr)`.

| Function | Description |
|----------|-------------|
| `zephyr.msleep(ms)` | Sleep for `ms` milliseconds |
| `zephyr.printk(msg)` | Kernel print |
| `zephyr.log_inf(msg)` | Log at INFO level |
| `zephyr.log_wrn(msg)` | Log at WARNING level |
| `zephyr.log_dbg(msg)` | Log at DEBUG level |
| `zephyr.log_err(msg)` | Log at ERROR level |

### `zbus` library

Loaded with `LUA_REQUIRE(zbus)`. Channels and observers are registered in the
setup hook with `LUA_ZBUS_CHAN_DECLARE` / `LUA_ZBUS_OBS_DECLARE`.

| Method | Description |
|--------|-------------|
| `chan:pub(table, timeout_ms)` | Publish a Lua table to a zbus channel |
| `chan:read(timeout_ms)` | Read the current channel value as a Lua table |
| `obs:wait_msg(timeout_ms)` | Block until a message arrives; returns `err, chan, table` |

### `fs` library

Loaded with `LUA_REQUIRE(fs)`. Requires `CONFIG_LUA_FS=y`.

| Function | Description |
|----------|-------------|
| `fs.dofile(path)` | Load and execute a Lua script from the filesystem |
| `fs.loadfile(path)` | Load a script without executing (returns a function) |
| `fs.list([path])` | List files in a directory |

## Configuration

All options live under `Kconfig.lua_module`.

| Option | Default | Description |
|--------|---------|-------------|
| `CONFIG_LUA` | — | Enable Lua support (selects zbus) |
| `CONFIG_LUA_REPL` | `n` | Enable interactive Lua shell (selects Zephyr shell) |
| `CONFIG_LUA_REPL_LINE_SIZE` | `256` | Maximum REPL input line length |
| `CONFIG_LUA_THREAD_STACK_SIZE` | `2048` | Stack size (bytes) for generated Lua threads |
| `CONFIG_LUA_THREAD_HEAP_SIZE` | `32768` | Heap size (bytes) for generated Lua threads |
| `CONFIG_LUA_THREAD_PRIORITY` | `7` | Cooperative priority of generated Lua threads |
| `CONFIG_LUA_PRECOMPILE` | `n` | Precompile Lua scripts to bytecode at build time |
| `CONFIG_LUA_PRECOMPILE_ONLY` | `n` | Exclude the Lua parser from the target (saves ~15-20 KB) |
| `CONFIG_LUA_FS` | `n` | Enable filesystem support for Lua scripts |
| `CONFIG_LUA_FS_MOUNT_POINT` | `"/lfs"` | Filesystem mount point prefix |
| `CONFIG_LUA_FS_MAX_FILE_SIZE` | `4096` | Maximum Lua script file size (bytes) |
| `CONFIG_LUA_FS_SHELL` | `n` | Enable `lua_fs` shell commands (list, cat, write, delete, run, stat) |

## Architecture

Each `add_lua_thread()` (or bytecode/fs variant) generates a self-contained
Zephyr thread with:

- A dedicated **sys_heap** (`CONFIG_LUA_THREAD_HEAP_SIZE`)
- A dedicated **thread stack** (`CONFIG_LUA_THREAD_STACK_SIZE`)
- A custom **Lua allocator** backed by the thread's heap
- A **setup hook** (`<script>_lua_setup`) for registering libraries

Scripts interact with the rest of the system exclusively through **zbus
channels**, keeping the Lua sandbox isolated from direct hardware access.

![Architecture](https://github.com/user-attachments/assets/c6bc2fc3-6ba3-45a1-98c0-98e4dd28f1bf)

## License

Apache-2.0
