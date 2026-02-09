# Conversation Log: Descriptor-based Lua ↔ C struct conversion for zbus

**Active Claude working time: ~30 minutes** (excludes user response time)

| Phase                                          | Approx. Time |
| ---------------------------------------------- | ------------ |
| Codebase exploration & file reading            | ~2 min       |
| Code writing (header, impl, modifications)     | ~5 min       |
| Build/debug cycle (4 builds + linker research) | ~15 min      |
| Testing (QEMU + twister + other samples)       | ~5 min       |
| Formatting, commit, CLAUDE.md update           | ~3 min       |

---

## Raw Prompt

```console
I would like to create a way to improve the `msg_struct_to_lua_table` and `lua_table_to_msg_struct` functions. I would like to do something more automatic, less error prone. Probably we could use something similar to https://docs.zephyrproject.org/latest/services/serialization/json.html where we create a descriptor and with that we could
  make both encode and decode from lua tables to C structs. Let's plan that?
```

# Plan: Descriptor-based Lua ↔ C struct conversion for zbus

## Context

`msg_struct_to_lua_table` / `lua_table_to_msg_struct` are `__weak` functions overridden with manual if/else chains, channel pointer comparisons, and hand-coded field extraction. Error-prone, verbose, doesn't scale. Inspired by Zephyr's `json_obj_descr`, we'll create a declarative descriptor system with automatic encode/decode.

## Design

### Types

```c
enum lua_msg_field_type {
    LUA_MSG_TYPE_INT,        /* signed int (1/2/4/8 bytes) → lua_pushinteger */
    LUA_MSG_TYPE_UINT,       /* unsigned int (1/2/4/8 bytes) → lua_pushinteger */
    LUA_MSG_TYPE_NUMBER,     /* float/double → lua_pushnumber */
    LUA_MSG_TYPE_STRING,     /* const char* → lua_pushstring */
    LUA_MSG_TYPE_BOOL,       /* bool → lua_pushboolean */
    LUA_MSG_TYPE_OBJECT,     /* nested struct → nested Lua table */
};

struct lua_msg_field_descr {
    const char *field_name;
    enum lua_msg_field_type type;
    uint16_t offset;         /* offsetof(struct, field) */
    uint8_t size;            /* sizeof field (for primitives) */
    /* For LUA_MSG_TYPE_OBJECT: */
    const struct lua_msg_field_descr *sub_fields;
    size_t sub_field_count;
};

struct lua_msg_descr {
    const struct zbus_channel *chan;
    const struct lua_msg_field_descr *fields;
    size_t field_count;
    size_t msg_size;
};
```

### Macros

```c
/* Primitive field */
#define LUA_MSG_FIELD(_struct, _field, _type)

/* Nested struct field */
#define LUA_MSG_FIELD_OBJECT(_struct, _field, _sub_fields)

/* Register channel↔descriptor (uses STRUCT_SECTION_ITERABLE) */
#define LUA_ZBUS_MSG_DESCR_DEFINE(_name, _chan, _struct, _fields)
```

### Before → After

**Before** (~40 lines manual code):

```c
int msg_struct_to_lua_table(lua_State *L, const struct zbus_channel *chan, void *message) {
    lua_newtable(L);
    if (chan == &chan_acc_data_consumed) {
        struct msg_acc_data_consumed *msg = message;
        LUA_TABLE_SET("type", string, "msg_acc_data_consumed");
        LUA_TABLE_SET("count", integer, msg->count);
    } else if (chan == &chan_version) { /* ... */ }
    return 1;
}
size_t lua_table_to_msg_struct(lua_State *L, void *message) {
    const char *type = LUA_TABLE_GET("type", string);
    if (strcmp(type, "msg_acc_data") == 0) { /* ... */ }
    return 1;  /* BUG: should return ret */
}
```

**After** (~20 lines declarative, zero logic):

```c
static const struct lua_msg_field_descr acc_data_fields[] = {
    LUA_MSG_FIELD(struct msg_acc_data, x, LUA_MSG_TYPE_INT),
    LUA_MSG_FIELD(struct msg_acc_data, y, LUA_MSG_TYPE_INT),
    LUA_MSG_FIELD(struct msg_acc_data, z, LUA_MSG_TYPE_INT),
};
LUA_ZBUS_MSG_DESCR_DEFINE(acc_data_descr, chan_acc_data,
                           struct msg_acc_data, acc_data_fields);
/* ... similar for other channels ... */
```

No weak-function overrides. No `type` discriminator field in Lua tables.

### Registration: Zephyr iterable sections

`LUA_ZBUS_MSG_DESCR_DEFINE` uses `STRUCT_SECTION_ITERABLE(lua_msg_descr, ...)`. Descriptors from any compilation unit collected by linker. Generic implementation iterates section matching channel pointer.

### Nested struct support

```c
/* Example: nested struct */
struct inner { int a; int b; };
struct outer { int id; struct inner data; };

static const struct lua_msg_field_descr inner_fields[] = {
    LUA_MSG_FIELD(struct inner, a, LUA_MSG_TYPE_INT),
    LUA_MSG_FIELD(struct inner, b, LUA_MSG_TYPE_INT),
};
static const struct lua_msg_field_descr outer_fields[] = {
    LUA_MSG_FIELD(struct outer, id, LUA_MSG_TYPE_INT),
    LUA_MSG_FIELD_OBJECT(struct outer, data, inner_fields),
};
```

Encode: creates nested Lua table `{id=1, data={a=2, b=3}}`.
Decode: reads nested table back into C struct.

### Backward compat via Kconfig

- `CONFIG_LUA_ZBUS_MSG_DESCR=y` → strong implementations override weak defaults
- `CONFIG_LUA_ZBUS_MSG_DESCR=n` (default) → old weak-function pattern preserved

### Signature change: `lua_table_to_msg_struct`

Current: `size_t lua_table_to_msg_struct(lua_State *L, void *message)`
New: `size_t lua_table_to_msg_struct(lua_State *L, const struct zbus_channel *chan, void *message)`

Channel needed for descriptor lookup. Applies regardless of Kconfig.

## Implementation steps

### 1. Create `lua_zephyr/include/lua_zephyr/lua_msg_descr.h`

- Enum, structs, macros defined above
- `LUA_MSG_FIELD`, `LUA_MSG_FIELD_OBJECT`, `LUA_ZBUS_MSG_DESCR_DEFINE`

### 2. Create `lua_zephyr/src/lua_zephyr/lua_msg_descr.c`

- `lua_msg_descr_find(chan)` — iterate iterable section, match by channel ptr
- `lua_msg_descr_to_table(L, fields, field_count, message)` — recursive: for each field, push to Lua table; for OBJECT type, recurse with sub_fields creating nested table
- `lua_msg_descr_from_table(L, fields, field_count, message, table_idx)` — recursive: for each field, extract from Lua table; for OBJECT type, recurse into nested table
- Strong `msg_struct_to_lua_table()` — find descriptor → encode → return table (or nil)
- Strong `lua_table_to_msg_struct()` — find descriptor → decode → return msg_size (or 0)

### 3. Modify `lua_zephyr/src/lua_zephyr/lua_zbus.c`

- `lua_table_to_msg_struct` weak: add `chan` param
- `chan_pub()`: pass `*chan` to call

### 4. Modify `lua_zephyr/include/lua_zephyr/zbus.h`

- Update extern declarations if any (currently none for the weak functions, but good to document the new signature)

### 5. Modify `lua_zephyr/Kconfig.lua_module`

- Add `CONFIG_LUA_ZBUS_MSG_DESCR` (bool, depends on LUA && ZBUS)

### 6. Modify `lua_zephyr/CMakeLists.txt`

- Add `lua_msg_descr.c` conditionally on `CONFIG_LUA_ZBUS_MSG_DESCR`

### 7. Update `samples/producer_consumer/`

- `prj.conf`: `CONFIG_LUA_ZBUS_MSG_DESCR=y`
- `src/producer_lua.c`: replace weak-function overrides with descriptor definitions
- `src/producer.lua`: remove `type` field from `acc_data` table

## Files

| Action | File                                            |
| ------ | ----------------------------------------------- |
| Create | `lua_zephyr/include/lua_zephyr/lua_msg_descr.h` |
| Create | `lua_zephyr/src/lua_zephyr/lua_msg_descr.c`     |
| Modify | `lua_zephyr/src/lua_zephyr/lua_zbus.c`          |
| Modify | `lua_zephyr/Kconfig.lua_module`                 |
| Modify | `lua_zephyr/CMakeLists.txt`                     |
| Modify | `samples/producer_consumer/src/producer_lua.c`  |
| Modify | `samples/producer_consumer/src/producer.lua`    |
| Modify | `samples/producer_consumer/prj.conf`            |

## Verification

1. `west build -d build -b qemu_cortex_m3 samples/producer_consumer` — compiles
2. `west build -t run` — same output as before (minus "type" field usage)
3. `west twister -T samples/producer_consumer -p qemu_x86` — tests pass
4. Disable `CONFIG_LUA_ZBUS_MSG_DESCR=n` + restore old weak overrides → still builds
