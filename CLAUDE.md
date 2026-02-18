# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# Restrictions

Lua core sources live in a git submodule (`lua/` → `github.com/lua/lua` at `v5.5.0`). Do not modify files inside the submodule. Only modify files in `src/`, `include/`, and `host_tools/`.

## Project

Zephyr RTOS module integrating Lua 5.5.0 as a first-class scripting engine for embedded systems. Lua scripts run in dedicated threads with isolated heaps; IPC happens via zbus channels.

## Build Commands

Uses `just` (justfile) with West/CMake underneath. Default board: `mps2/an385`.

| Command            | Alias     | Description                                          |
| ------------------ | --------- | ---------------------------------------------------- |
| `just build`       | `just b`  | Build (`west build -d ./build -b mps2/an385 app`)    |
| `just rebuild`     | `just bb` | Rebuild without full reconfigure                     |
| `just clean`       | `just c`  | Remove build directory                               |
| `just run`         | `just r`  | Build and run in QEMU                                |
| `just config`      |           | Open menuconfig                                      |
| `just debugserver` | `just ds` | Start QEMU GDB server                                |
| `just attach`      | `just da` | Attach GDB to running debug session                  |
| `just test`        |           | Run twister test suite (`-p mps2/an385 -T samples`)  |
| `just format`      |           | Format project C/H files (excludes core Lua sources) |

### Running Tests

Tests use Zephyr's twister harness. Each sample has a `sample.yaml` defining console-based test expectations:

```sh
west twister -p mps2/an385 -T samples -O /tmp/lua_tests/
```

### Formatting

```sh
just format          # Runs clang-format on src/ include/ and samples/ C/H files
clang-format -i <file>   # Uses .clang-format (Zephyr-aligned LLVM, 8-space indent, 100-col limit)
```

## Architecture

### Module Structure

- **Lua 5.5.0 core** — Git submodule (`lua/`) from `github.com/lua/lua` at tag `v5.5.0`
- **Zephyr integration** (`src/`):
  - `luaz_utils.c` — Custom `sys_heap` allocator, `luaz_openlibs()`, minimal `require()`, `luaz_print_mem_usage()`, kernel API bindings (`zephyr.msleep`, `zephyr.printk`, `zephyr.log_*`)
  - `luaz_zbus.c` — zbus channel/observer Lua bindings (pub, read, wait_msg)
  - `luaz_msg_descr.c` — Descriptor-based Lua↔C struct conversion (helper library)
  - `luaz_repl.c` — Interactive Lua shell (enabled via `CONFIG_LUA_REPL`)
  - `luaz_fs.c` — Filesystem support: `dofile`, `loadfile`, `list`, `write_file` (enabled via `CONFIG_LUA_FS`)
  - `luaz_fs_shell.c` — Shell commands for managing Lua scripts on FS: list, cat, write, delete, run, stat (enabled via `CONFIG_LUA_FS_SHELL`)
  - `luaz_parser_stubs.c` — Parser stubs for bytecode-only builds (enabled via `CONFIG_LUA_PRECOMPILE_ONLY`)

### CMake API (`luaz.cmake`)

Single file included **twice** during a build:

1. **Pre-Zephyr** (by user CMakeLists.txt) — thread definitions + Kconfig generation
2. **Post-Zephyr** (by module CMakeLists.txt) — code generation + host luac build

**Pre-Zephyr macros** (call before `find_package(Zephyr)`):

- `luaz_define_source_thread(path)` — Define a source-embedded thread
- `luaz_define_bytecode_thread(path)` — Define a bytecode thread
- `luaz_define_fs_thread(fs_path)` — Define a filesystem-backed thread

Each generates a per-thread Kconfig fragment with `<NAME>_LUA_THREAD_STACK_SIZE`, `_HEAP_SIZE`, `_PRIORITY`.

**Post-Zephyr functions** (call after `find_package(Zephyr)`):

- `luaz_generate_threads()` — Generate all defined threads (processes `LUAZ_SOURCE_THREADS`, `LUAZ_BYTECODE_THREADS`, `LUAZ_FS_THREADS` lists)
- `luaz_add_file(path)` — Embed a `.lua` file as a C string header (via `luaz_gen.py`)
- `luaz_add_bytecode_file(path)` — Embed precompiled bytecode as a C `uint8_t[]` header (requires `CONFIG_LUA_PRECOMPILE`)
- `luaz_add_fs_file(src [name])` — Register a Lua file for embedding and writing to the FS at boot (requires `CONFIG_LUA_FS`)

### Thread Model

Each generated thread gets:

- Dedicated `sys_heap` (per-thread `CONFIG_<NAME>_LUA_THREAD_HEAP_SIZE`, defaults to `CONFIG_LUA_THREAD_HEAP_SIZE` = 32KB)
- Dedicated stack (per-thread `CONFIG_<NAME>_LUA_THREAD_STACK_SIZE`, defaults to `CONFIG_LUA_THREAD_STACK_SIZE` = 2KB)
- Custom Lua allocator backed by the thread's heap
- `luaz_openlibs(L)` called automatically — registers `require()` and preloads all enabled libraries
- A weak setup hook (`<script>_lua_setup`) for registering zbus channels/observers
- `luaz_print_mem_usage()` called after script exits — prints heap and stack usage table (requires `CONFIG_SYS_HEAP_RUNTIME_STATS` for heap, `CONFIG_INIT_STACKS` + `CONFIG_THREAD_STACK_INFO` for stack)

**Variants:**

- **Source thread** (`luaz_define_source_thread`) — script embedded as C string, parsed at startup
- **Bytecode thread** (`luaz_define_bytecode_thread`) — script precompiled, parser can be stripped
- **Filesystem thread** (`luaz_define_fs_thread`) — script loaded from FS path at runtime

### Lua API

Scripts use `require("zephyr")` to access kernel bindings. zbus and fs are nested subtables:

- `zephyr.msleep`, `zephyr.printk`, `zephyr.log_inf`, `zephyr.log_wrn`, `zephyr.log_dbg`, `zephyr.log_err`
- `zephyr.zbus` — zbus bindings (when `CONFIG_LUA_LIB_ZBUS=y`): `chan:pub`, `chan:read`, `obs:wait_msg`
- `zephyr.fs` — filesystem bindings (when `CONFIG_LUA_FS=y`): `fs.dofile`, `fs.loadfile`, `fs.list`

Channels/observers are registered in the setup hook via `LUA_REQUIRE_ZBUS_CHAN(chan)` / `LUA_REQUIRE_ZBUS_OBS(obs)`.

### zbus Integration

The descriptor system (`luaz_msg_descr.h`) handles Lua table ↔ C struct conversion for zbus messages. Descriptors are stored as zbus channel `user_data` for O(1) lookup. Pass `LUA_ZBUS_MSG_DESCR(type, fields)` as the `_user_data` argument to `ZBUS_CHAN_DEFINE` — it creates a compound-literal descriptor inline. The internal conversion functions in `luaz_zbus.c` check `zbus_chan_user_data()` and call `lua_msg_descr_to_table`/`lua_msg_descr_from_table` automatically. Nested struct support via `LUA_MSG_TYPE_OBJECT` / `LUA_MSG_FIELD_OBJECT`.

### nanopb Descriptor Bridge (`luaz_msg_descr_pb.h`)

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

| Option                           | Default  | Description                                                        |
| -------------------------------- | -------- | ------------------------------------------------------------------ |
| `CONFIG_LUA`                     | —        | Enable Lua support (selects zbus)                                  |
| `CONFIG_LUA_REPL`                | `n`      | Enable interactive Lua shell                                       |
| `CONFIG_LUA_REPL_LINE_SIZE`      | `256`    | Maximum REPL input line length                                     |
| `CONFIG_LUA_THREAD_STACK_SIZE`   | `2048`   | Default stack size for generated Lua threads                       |
| `CONFIG_LUA_THREAD_HEAP_SIZE`    | `32768`  | Default heap size for generated Lua threads                        |
| `CONFIG_LUA_THREAD_PRIORITY`     | `7`      | Default priority of generated Lua threads                          |
| `CONFIG_LUA_LIBS_ALL`            | `n`      | Preload all standard Lua libraries + zbus                          |
| `CONFIG_LUA_LIB_STRING`          | if ALL   | Lua string library                                                 |
| `CONFIG_LUA_LIB_TABLE`           | if ALL   | Lua table library                                                  |
| `CONFIG_LUA_LIB_MATH`            | if ALL   | Lua math library                                                   |
| `CONFIG_LUA_LIB_COROUTINE`       | if ALL   | Lua coroutine library                                              |
| `CONFIG_LUA_LIB_UTF8`            | if ALL   | Lua utf8 library                                                   |
| `CONFIG_LUA_LIB_DEBUG`           | if ALL   | Lua debug library                                                  |
| `CONFIG_LUA_LIB_ZBUS`            | if ALL   | Include zbus bindings as `zephyr.zbus` subtable                    |
| `CONFIG_LUA_PRECOMPILE`          | `n`      | Precompile scripts to bytecode at build time                       |
| `CONFIG_LUA_PRECOMPILE_ONLY`     | `n`      | Exclude Lua parser (~15-20 KB savings)                             |
| `CONFIG_LUA_EXTRA_OPTIMIZATIONS` | `n`      | Reduce internal data structure sizes (experimental, bytecode-only) |
| `CONFIG_LUA_FS`                  | `n`      | Enable filesystem support                                          |
| `CONFIG_LUA_FS_MOUNT_POINT`      | `"/lfs"` | Filesystem mount point prefix                                      |
| `CONFIG_LUA_FS_MAX_FILE_SIZE`    | `4096`   | Maximum script file size (bytes)                                   |
| `CONFIG_LUA_FS_SHELL`            | `n`      | Enable `lua_fs` shell commands                                     |

Per-thread overrides: each `luaz_define_*_thread()` generates `CONFIG_<SCRIPT>_LUA_THREAD_STACK_SIZE`, `_HEAP_SIZE`, `_PRIORITY` options that default to the global values above.

## Code Style

- C formatting: `.clang-format` (Zephyr style — LLVM base, Linux braces, 8-space indent, tabs for continuation, 100-col limit)
- Use `/* */` for comments, `/** */` for Doxygen docs
- Always use braces with `if`/`else`, even for single-line bodies
- Commit messages: conventional commits (`fix:`, `feat:`, `refactor:`, `chore:`)
