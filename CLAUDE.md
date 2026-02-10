# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# Restrictions

We are not going to change and LUA source files. They are inside the @lua_zephyr/include and @lua_zephyr/src. The only exception is the @lua_zephyr/src/lua_zephyr and @lua_zephyr/include/lua_zephyr folders.

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

### Running Tests

Tests use Zephyr's twister harness. Each sample has a `sample.yaml` defining console-based test expectations:

```sh
west twister -T samples/ -p qemu_x86
```

### Formatting

```sh
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

### CMake Functions (`lua.cmake`)

- `add_lua_file(path)` — Embed a `.lua` file as a C string header (via `lua_cat.py`)
- `add_lua_thread(path)` — Generate a complete Lua thread with heap, state, and script execution (via `lua_thread.c.in` template)

### Thread Model

Each `add_lua_thread()` call generates a Zephyr thread with:

- Dedicated `sys_heap` (`CONFIG_LUA_THREAD_HEAP_SIZE`, default 32KB)
- Dedicated stack (`CONFIG_LUA_THREAD_STACK_SIZE`, default 2KB)
- Custom Lua allocator backed by the thread's heap

### zbus Integration

Weakly-defined conversion hooks allow applications to customize message serialization:

- `msg_struct_to_lua_table(L, chan, message)` — C struct → Lua table
- `lua_table_to_msg_struct(L, chan, message)` — Lua table → C struct

The descriptor system (`lua_msg_descr.h`) stores descriptors as zbus channel `user_data` for O(1) lookup. Pass `LUA_ZBUS_MSG_DESCR(type, fields)` as the `_user_data` argument to `ZBUS_CHAN_DEFINE` — it creates a compound-literal descriptor inline. The `__weak` defaults in `lua_zbus.c` check `zbus_chan_user_data()` and call `lua_msg_descr_to_table`/`lua_msg_descr_from_table` automatically. Apps can still provide strong overrides. Nested struct support via `LUA_MSG_TYPE_OBJECT` / `LUA_MSG_FIELD_OBJECT`.

### Key Kconfig Options

`CONFIG_LUA`, `CONFIG_LUA_REPL`, `CONFIG_LUA_THREAD_STACK_SIZE`, `CONFIG_LUA_THREAD_HEAP_SIZE`, `CONFIG_LUA_THREAD_PRIORITY`

## Code Style

- C formatting: `.clang-format` (Zephyr style — LLVM base, Linux braces, 8-space indent, tabs for continuation, 100-col limit)
- Use `/* */` for comments, `/** */` for Doxygen docs
- Always use braces with `if`/`else`, even for single-line bodies
- Commit messages: conventional commits (`fix:`, `feat:`, `refactor:`, `chore:`)
