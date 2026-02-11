# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# Restrictions

We are not going to change any Lua source files. They are inside the @lua_zephyr/include and @lua_zephyr/src. The only exception is the @lua_zephyr/src/lua_zephyr and @lua_zephyr/include/lua_zephyr folders.

## Project

Zephyr RTOS module integrating Lua 5.4.7 as a first-class scripting engine for embedded systems. Lua scripts run in dedicated threads with isolated heaps; IPC happens via zbus channels.

## Build Commands

Uses `just` (justfile) with West/CMake underneath. Default board: `qemu_cortex_m3`.

| Command            | Alias     | Description                                           |
| ------------------ | --------- | ----------------------------------------------------- |
| `just build`       | `just b`  | Build (`west build -d ./build -b qemu_cortex_m3 app`) |
| `just rebuild`     | `just bb` | Rebuild without full reconfigure                      |
| `just clean`       | `just c`  | Remove build directory                                |
| `just run`         | `just r`  | Build and run in QEMU                                 |
| `just config`      |           | Open menuconfig                                       |
| `just debugserver` | `just ds` | Start QEMU GDB server                                 |
| `just attach`      | `just da` | Attach GDB to running debug session                   |
| `just test`        |           | Run twister test suite (`-p mps2/an385 -T samples`)   |
| `just format`      |           | Format project C/H files (excludes core Lua sources)  |

### Running Tests

Tests use Zephyr's twister harness. Each sample has a `sample.yaml` defining console-based test expectations:

```sh
west twister -p mps2/an385 -T samples -O /tmp/lua_tests/
```

### Formatting

```sh
just format          # Runs clang-format on lua_zephyr/ and samples/ C/H files
clang-format -i <file>   # Uses .clang-format (Zephyr-aligned LLVM, 8-space indent, 100-col limit)
```

## Architecture

### Module Structure (`lua_zephyr/`)

- **Lua 5.4.7 core** — Full standard implementation (`l*.c` files in `src/`)
- **Zephyr integration** (`src/lua_zephyr/`):
  - `utils.c` — Custom `sys_heap` allocator, kernel API bindings (`zephyr.msleep`, `zephyr.printk`, `zephyr.log_*`)
  - `lua_zbus.c` — zbus channel/observer Lua bindings (pub, read, wait_msg)
  - `lua_msg_descr.c` — Descriptor-based Lua↔C struct conversion (helper library)
  - `lua_repl.c` — Interactive Lua shell (enabled via `CONFIG_LUA_REPL`)
  - `lua_fs.c` — Filesystem support: `dofile`, `loadfile`, `list`, `write_file` (enabled via `CONFIG_LUA_FS`)
  - `lua_fs_shell.c` — Shell commands for managing Lua scripts on FS: list, cat, write, delete, run, stat (enabled via `CONFIG_LUA_FS_SHELL`)

### CMake Functions (`lua.cmake`)

- `add_lua_file(path)` — Embed a `.lua` file as a C string header (via `lua_cat.py`)
- `add_lua_thread(path)` — Generate a Lua thread with heap, state, and embedded source script (via `lua_thread.c.in`)
- `add_lua_bytecode_file(path)` — Embed precompiled bytecode as a C `uint8_t[]` header (requires `CONFIG_LUA_PRECOMPILE`)
- `add_lua_bytecode_thread(path)` — Generate a Lua thread that loads precompiled bytecode (requires `CONFIG_LUA_PRECOMPILE`)
- `add_lua_fs_file(src [name])` — Register a Lua file for embedding and writing to the filesystem at boot (requires `CONFIG_LUA_FS`)
- `add_lua_fs_thread(fs_path)` — Generate a Lua thread that loads its script from the filesystem at runtime (requires `CONFIG_LUA_FS`)

### Thread Model

Each `add_lua_thread()` call (and its bytecode/fs variants) generates a Zephyr thread with:

- Dedicated `sys_heap` (`CONFIG_LUA_THREAD_HEAP_SIZE`, default 32KB)
- Dedicated stack (`CONFIG_LUA_THREAD_STACK_SIZE`, default 2KB)
- Custom Lua allocator backed by the thread's heap
- A setup hook (`<script>_lua_setup`) for registering libraries

**Variants:**
- **Source thread** (`add_lua_thread`) — script embedded as C string, parsed at startup
- **Bytecode thread** (`add_lua_bytecode_thread`) — script precompiled, parser can be stripped
- **Filesystem thread** (`add_lua_fs_thread`) — script loaded from FS path at runtime

### zbus Integration

Weakly-defined conversion hooks allow applications to customize message serialization:

- `msg_struct_to_lua_table(L, chan, message)` — C struct → Lua table
- `lua_table_to_msg_struct(L, chan, message)` — Lua table → C struct

The descriptor system (`lua_msg_descr.h`) stores descriptors as zbus channel `user_data` for O(1) lookup. Pass `LUA_ZBUS_MSG_DESCR(type, fields)` as the `_user_data` argument to `ZBUS_CHAN_DEFINE` — it creates a compound-literal descriptor inline. The `__weak` defaults in `lua_zbus.c` check `zbus_chan_user_data()` and call `lua_msg_descr_to_table`/`lua_msg_descr_from_table` automatically. Apps can still provide strong overrides. Nested struct support via `LUA_MSG_TYPE_OBJECT` / `LUA_MSG_FIELD_OBJECT`.

### nanopb Descriptor Bridge (`lua_msg_descr_pb.h`)

`LUA_PB_DESCR_DEFINE(_name)` auto-generates `lua_msg_field_descr` arrays from nanopb `FIELDLIST` X-macros, making `.proto` the single source of truth.

Nested MESSAGE fields are resolved automatically via nanopb's `<parent_t>_<field>_MSGTYPE` macros. **Child descriptors must be defined before parents** (leaf-first ordering):

```c
/* msg_acc_data has no dependencies — define first */
LUA_PB_DESCR_DEFINE(msg_acc_data);
ZBUS_CHAN_DEFINE(chan_acc_data, struct msg_acc_data, NULL,
                 LUA_PB_DESCR_REF(msg_acc_data), ...);

/* msg_sensor_config contains a msg_acc_data "offset" field — define after */
LUA_PB_DESCR_DEFINE(msg_sensor_config);
ZBUS_CHAN_DEFINE(chan_sensor_config, struct msg_sensor_config, NULL,
                 LUA_PB_DESCR_REF(msg_sensor_config), ...);
```

Deeper nesting (3+ levels) works the same way — always define leaf types first. `LUA_PB_DESCR_REF(_name)` returns a `void *` suitable for `ZBUS_CHAN_DEFINE` `_user_data`.

### Kconfig Options

| Option | Default | Description |
|--------|---------|-------------|
| `CONFIG_LUA` | — | Enable Lua support (selects zbus) |
| `CONFIG_LUA_REPL` | `n` | Enable interactive Lua shell |
| `CONFIG_LUA_REPL_LINE_SIZE` | `256` | Maximum REPL input line length |
| `CONFIG_LUA_THREAD_STACK_SIZE` | `2048` | Stack size for generated Lua threads |
| `CONFIG_LUA_THREAD_HEAP_SIZE` | `32768` | Heap size for generated Lua threads |
| `CONFIG_LUA_THREAD_PRIORITY` | `7` | Priority of generated Lua threads |
| `CONFIG_LUA_PRECOMPILE` | `n` | Precompile scripts to bytecode at build time |
| `CONFIG_LUA_PRECOMPILE_ONLY` | `n` | Exclude Lua parser (~15-20 KB savings) |
| `CONFIG_LUA_FS` | `n` | Enable filesystem support |
| `CONFIG_LUA_FS_MOUNT_POINT` | `"/lfs"` | Filesystem mount point prefix |
| `CONFIG_LUA_FS_MAX_FILE_SIZE` | `4096` | Maximum script file size (bytes) |
| `CONFIG_LUA_FS_SHELL` | `n` | Enable `lua_fs` shell commands |

## Code Style

- C formatting: `.clang-format` (Zephyr style — LLVM base, Linux braces, 8-space indent, tabs for continuation, 100-col limit)
- Use `/* */` for comments, `/** */` for Doxygen docs
- Always use braces with `if`/`else`, even for single-line bodies
- Commit messages: conventional commits (`fix:`, `feat:`, `refactor:`, `chore:`)
