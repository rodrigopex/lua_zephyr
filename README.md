# Lua for Zephyr RTOS

> **Alpha** — API may change. Feedback welcome!

A [Zephyr RTOS](https://zephyrproject.org/) module that integrates
[Lua 5.5.0](https://www.lua.org/) as a first-class scripting engine for
embedded systems. Lua scripts run in **dedicated threads** with **isolated
heaps**; inter-thread communication happens through **zbus channels**, keeping
scripting sandboxed and deterministic.

![Architecture](https://github.com/user-attachments/assets/c6bc2fc3-6ba3-45a1-98c0-98e4dd28f1bf)

Each Lua thread gets its own **sys_heap**, **stack**, and **allocator** — fully
isolated from every other thread. Scripts talk to the rest of the firmware
exclusively through **zbus**, so adding or removing a Lua script never
destabilises the system.

## Features

**Script loading** — three ways to get Lua code onto a target:

- **Embedded source** — compiled into the firmware image as C strings
- **Precompiled bytecode** — strip the parser at build time, saving ~15-20 KB flash
- **Filesystem** — load scripts from LittleFS (or any Zephyr FS) at runtime

**System integration:**

- **zbus bindings** — publish, read, and subscribe to channels directly from Lua
- **Message descriptors** — automatic Lua table ↔ C struct conversion (manual or nanopb-generated)
- **Kernel API** — `msleep`, `printk`, structured logging (`log_inf`, `log_wrn`, `log_dbg`, `log_err`)
- **Interactive REPL** — Lua shell over the Zephyr console

## Table of Contents

- [Quick Start](#quick-start)
- [Samples](#samples)
- [How It Works](#how-it-works)
- [CMake API](#cmake-api)
- [Lua API](#lua-api)
- [Message Descriptors](#message-descriptors)
- [Configuration](#configuration)
- [License](#license)

## Quick Start

### Prerequisites

- A [Zephyr workspace](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) (west, SDK, toolchain)
- [`just`](https://github.com/casey/just) command runner (optional but recommended)

### 1. Add the module

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

### 2. Write a Lua script

```lua
-- src/my_script.lua
local z = zephyr

z.printk("Hello from Lua!")
z.msleep(1000)
z.printk("Done.")
```

### 3. Wire up the setup hook

Each `add_lua_thread("src/my_script.lua")` generates a thread that calls
`my_script_lua_setup` before running the script — use it to register
the libraries your script needs:

```c
#include <lua.h>
#include <luaz_utils.h>

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

### 4. Enable Lua and build

```
# prj.conf
CONFIG_LUA=y
```

```sh
just build          # west build -b mps2/an385 app
just run            # west build -t run
```

## Samples

Every sample includes a `sample.yaml` for automated testing with
[twister](https://docs.zephyrproject.org/latest/develop/test/twister.html).

| Sample                                                 | Description                              | Key Features                                        |
| ------------------------------------------------------ | ---------------------------------------- | --------------------------------------------------- |
| [`hello_world`](samples/hello_world)                   | Basic Lua thread + embedded script       | `add_lua_thread`, `add_lua_file`, REPL, logging     |
| [`hello_world_bytecode`](samples/hello_world_bytecode) | Bytecode-only variant                    | `add_lua_bytecode_thread`, parser stripped          |
| [`producer_consumer`](samples/producer_consumer)       | zbus pub/sub between Lua and C           | nanopb descriptors, nested structs, bytecode        |
| [`littlefs`](samples/littlefs)                         | Scripts loaded from LittleFS at runtime  | `add_lua_fs_thread`, `add_lua_fs_file`, `fs.dofile` |
| [`heavy`](samples/heavy)                               | Stress test with dynamic code generation | Heap usage tracking, recursion, string ops          |

```sh
# Run a single sample
just build project_dir=samples/hello_world
just run

# Run the full test suite
just test           # west twister -p mps2/an385 -T samples
```

## How It Works

### Thread model

Each `add_lua_thread()` (or its bytecode / filesystem variant) generates a
self-contained Zephyr thread with:

- A dedicated **sys_heap** (`CONFIG_LUA_THREAD_HEAP_SIZE`, default 32 KB)
- A dedicated **thread stack** (`CONFIG_LUA_THREAD_STACK_SIZE`, default 2 KB)
- A custom **Lua allocator** backed by that heap
- A **setup hook** (`<script>_lua_setup`) for registering libraries

There are three thread flavours:

| Variant        | CMake function            | Script location                      |
| -------------- | ------------------------- | ------------------------------------ |
| **Source**     | `add_lua_thread`          | Embedded as C string, parsed at boot |
| **Bytecode**   | `add_lua_bytecode_thread` | Precompiled, parser can be stripped  |
| **Filesystem** | `add_lua_fs_thread`       | Loaded from FS path at runtime       |

### zbus integration

Scripts interact with the rest of the system exclusively through
[zbus](https://docs.zephyrproject.org/latest/services/zbus/index.html)
channels. Conversion between Lua tables and C structs is handled automatically
via **message descriptors** stored in `zbus_chan_user_data()` — see
[Message Descriptors](#message-descriptors) below.

### Module structure

| Path         | Contents                                                                    |
| ------------ | --------------------------------------------------------------------------- |
| `lua/`       | Lua 5.5.0 core (git submodule — **do not modify**)                          |
| `src/`       | Zephyr integration: allocator, kernel bindings, zbus, REPL, FS, descriptors |
| `include/`   | Public headers (`luaz_utils.h`, `luaz_zbus.h`, `luaz_fs.h`, …)              |
| `templates/` | `.c.in` / `.h.in` files used by the CMake code-gen functions                |
| `scripts/`   | Python helpers (`lua_cat.py`, `lua_compile.py`)                             |
| `samples/`   | Ready-to-build example applications                                         |

---

## CMake API

All functions are provided by `lua.cmake` (auto-included when the module is loaded).

### Thread generators

| Function                        | Description                                                                              |
| ------------------------------- | ---------------------------------------------------------------------------------------- |
| `add_lua_thread(path)`          | Embed a Lua source script and run it in a dedicated Zephyr thread                        |
| `add_lua_bytecode_thread(path)` | Same, but precompile to bytecode at build time (`CONFIG_LUA_PRECOMPILE`)                 |
| `add_lua_fs_thread(fs_path)`    | Generate a thread that loads its script from the filesystem at runtime (`CONFIG_LUA_FS`) |

### File embedders

| Function                      | Description                                                             |
| ----------------------------- | ----------------------------------------------------------------------- |
| `add_lua_file(path)`          | Embed a `.lua` file as a C `const char[]` header                        |
| `add_lua_bytecode_file(path)` | Embed precompiled bytecode as a C `uint8_t[]` header                    |
| `add_lua_fs_file(src [name])` | Register a Lua file for embedding and writing to the filesystem at boot |

## Lua API

### `zephyr` library

Loaded with `LUA_REQUIRE(zephyr)`.

| Function              | Description                 |
| --------------------- | --------------------------- |
| `zephyr.msleep(ms)`   | Sleep for `ms` milliseconds |
| `zephyr.printk(msg)`  | Kernel print                |
| `zephyr.log_inf(msg)` | Log at INFO level           |
| `zephyr.log_wrn(msg)` | Log at WARNING level        |
| `zephyr.log_dbg(msg)` | Log at DEBUG level          |
| `zephyr.log_err(msg)` | Log at ERROR level          |

### `zbus` library

Loaded with `LUA_REQUIRE(zbus)`. Channels and observers are registered in the
setup hook with `LUA_REQUIRE_ZBUS_CHAN` / `LUA_REQUIRE_ZBUS_OBS`.

| Method                        | Description                                               |
| ----------------------------- | --------------------------------------------------------- |
| `chan:pub(table, timeout_ms)` | Publish a Lua table to a zbus channel                     |
| `chan:read(timeout_ms)`       | Read the current channel value as a Lua table             |
| `obs:wait_msg(timeout_ms)`    | Block until a message arrives; returns `err, chan, table` |

### `fs` library

Loaded with `LUA_REQUIRE(fs)`. Requires `CONFIG_LUA_FS=y`.

Loading this library also replaces the global `dofile` and `loadfile` with
filesystem-backed versions, so scripts can use them transparently.

| Function            | Description                                          |
| ------------------- | ---------------------------------------------------- |
| `fs.dofile(path)`   | Load and execute a Lua script from the filesystem    |
| `fs.loadfile(path)` | Load a script without executing (returns a function) |
| `fs.list([path])`   | List files in a directory                            |

## Message Descriptors

The descriptor system provides **automatic Lua table ↔ C struct conversion**
for zbus messages. Descriptors are stored as zbus channel `user_data` for O(1)
lookup — the conversion functions in `luaz_zbus.c` use them automatically.

### Manual descriptors

Define field arrays and pass them inline to `ZBUS_CHAN_DEFINE`:

```c
#include <luaz_msg_descr.h>

struct sensor_data {
        int32_t x, y, z;
};

static const struct lua_msg_field_descr sensor_fields[] = {
        LUA_MSG_FIELD(struct sensor_data, x, LUA_MSG_TYPE_INT),
        LUA_MSG_FIELD(struct sensor_data, y, LUA_MSG_TYPE_INT),
        LUA_MSG_FIELD(struct sensor_data, z, LUA_MSG_TYPE_INT),
};

ZBUS_CHAN_DEFINE(chan_sensor, struct sensor_data, NULL,
                LUA_ZBUS_MSG_DESCR(struct sensor_data, sensor_fields),
                ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(0));
```

For nested structs use `LUA_MSG_FIELD_OBJECT`:

```c
LUA_MSG_FIELD_OBJECT(struct parent, child_field, child_fields),
```

Supported field types: `LUA_MSG_TYPE_INT`, `LUA_MSG_TYPE_UINT`,
`LUA_MSG_TYPE_NUMBER`, `LUA_MSG_TYPE_STRING`, `LUA_MSG_TYPE_STRING_BUF`,
`LUA_MSG_TYPE_BOOL`, `LUA_MSG_TYPE_OBJECT`.

### nanopb descriptor bridge

When using [nanopb](https://jpa.kapsi.fi/nanopb/), `luaz_msg_descr_pb.h`
auto-generates descriptors from nanopb `FIELDLIST` X-macros — the `.proto`
file becomes the single source of truth:

```c
#include <luaz_msg_descr_pb.h>
#include "channels.pb.h"

/* Child descriptors must be defined before parents (leaf-first) */
LUA_PB_DESCR_DEFINE(msg_acc_data);
ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL,
                LUA_PB_DESCR_REF(msg_acc_data),
                ZBUS_OBSERVERS_EMPTY, ZBUS_MSG_INIT(.x = 0));
```

Nested MESSAGE fields are resolved automatically via nanopb's
`<parent_t>_<field>_MSGTYPE` macros. See the [`producer_consumer`](samples/producer_consumer)
sample for a complete example.

## Configuration

All options live under `Kconfig.lua_module`.

| Option                         | Default  | Description                                                          |
| ------------------------------ | -------- | -------------------------------------------------------------------- |
| `CONFIG_LUA`                   | —        | Enable Lua support (selects zbus)                                    |
| `CONFIG_LUA_REPL`              | `n`      | Enable interactive Lua shell (selects Zephyr shell)                  |
| `CONFIG_LUA_REPL_LINE_SIZE`    | `256`    | Maximum REPL input line length                                       |
| `CONFIG_LUA_THREAD_STACK_SIZE` | `2048`   | Stack size (bytes) for generated Lua threads                         |
| `CONFIG_LUA_THREAD_HEAP_SIZE`  | `32768`  | Heap size (bytes) for generated Lua threads                          |
| `CONFIG_LUA_THREAD_PRIORITY`   | `7`      | Cooperative priority of generated Lua threads                        |
| `CONFIG_LUA_PRECOMPILE`        | `n`      | Precompile Lua scripts to bytecode at build time                     |
| `CONFIG_LUA_PRECOMPILE_ONLY`   | `n`      | Exclude the Lua parser from the target (saves ~15-20 KB)             |
| `CONFIG_LUA_FS`                | `n`      | Enable filesystem support for Lua scripts                            |
| `CONFIG_LUA_FS_MOUNT_POINT`    | `"/lfs"` | Filesystem mount point prefix                                        |
| `CONFIG_LUA_FS_MAX_FILE_SIZE`  | `4096`   | Maximum Lua script file size (bytes)                                 |
| `CONFIG_LUA_FS_SHELL`          | `n`      | Enable `lua_fs` shell commands (list, cat, write, delete, run, stat) |

## License

Apache-2.0
