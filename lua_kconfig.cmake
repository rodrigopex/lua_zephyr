# lua_kconfig.cmake — Pre-Zephyr Lua thread declaration and Kconfig generation.
#
# Include this file BEFORE find_package(Zephyr) and use the lua_add_*_thread()
# functions to declare Lua threads. Each call appends to the corresponding list
# variable and generates a per-thread Kconfig fragment with STACK_SIZE,
# HEAP_SIZE, and PRIORITY options that default to the global values.
#
# After find_package(Zephyr), call lua_generate_threads() to generate the
# thread C source files.
#
# Example:
#   include(${ZEPHYR_EXTRA_MODULES}/lua_kconfig.cmake)
#   lua_add_bytecode_thread(src/producer.lua)
#   lua_add_bytecode_thread(src/consumer.lua)
#   find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
#   project(my_app)
#   lua_generate_threads()

set(_LUA_KCONFIG_DIR "${CMAKE_BINARY_DIR}/Kconfig")
file(MAKE_DIRECTORY "${_LUA_KCONFIG_DIR}")

macro(_lua_write_thread_kconfig _name)
    string(TOUPPER "${_name}" _name_upper)
    set(_kconfig_file "${_LUA_KCONFIG_DIR}/Kconfig.lua_thread.${_name}")

    file(WRITE "${_kconfig_file}"
"config ${_name_upper}_LUA_THREAD_STACK_SIZE\n\
    int \"${_name} Lua thread stack size\"\n\
    default LUA_THREAD_STACK_SIZE\n\
    help\n\
      Stack size for the ${_name} Lua thread.\n\
\n\
config ${_name_upper}_LUA_THREAD_HEAP_SIZE\n\
    int \"${_name} Lua thread heap size\"\n\
    default LUA_THREAD_HEAP_SIZE\n\
    help\n\
      Heap size for the ${_name} Lua thread.\n\
\n\
config ${_name_upper}_LUA_THREAD_PRIORITY\n\
    int \"${_name} Lua thread priority\"\n\
    default LUA_THREAD_PRIORITY\n\
    help\n\
      Priority for the ${_name} Lua thread.\n"
    )
endmacro()

# lua_add_source_thread(FILE_NAME_PATH)
#
# Declare a source-embedded Lua thread. Appends to LUA_SOURCE_THREADS
# and generates the per-thread Kconfig fragment.
macro(lua_add_source_thread _path)
    list(APPEND LUA_SOURCE_THREADS "${_path}")
    set(_lua_thread_path "${_path}")
    cmake_path(GET _lua_thread_path FILENAME _lua_fname)
    cmake_path(REMOVE_EXTENSION _lua_fname OUTPUT_VARIABLE _lua_fname)
    _lua_write_thread_kconfig("${_lua_fname}")
endmacro()

# lua_add_bytecode_thread(FILE_NAME_PATH)
#
# Declare a bytecode Lua thread. Appends to LUA_BYTECODE_THREADS
# and generates the per-thread Kconfig fragment.
macro(lua_add_bytecode_thread _path)
    list(APPEND LUA_BYTECODE_THREADS "${_path}")
    set(_lua_thread_path "${_path}")
    cmake_path(GET _lua_thread_path FILENAME _lua_fname)
    cmake_path(REMOVE_EXTENSION _lua_fname OUTPUT_VARIABLE _lua_fname)
    _lua_write_thread_kconfig("${_lua_fname}")
endmacro()

# lua_add_fs_thread(SCRIPT_FS_PATH)
#
# Declare an FS-backed Lua thread. Appends to LUA_FS_THREADS
# and generates the per-thread Kconfig fragment.
macro(lua_add_fs_thread _path)
    list(APPEND LUA_FS_THREADS "${_path}")
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _lua_fname "${_path}")
    string(REGEX REPLACE "^_+" "" _lua_fname "${_lua_fname}")
    _lua_write_thread_kconfig("${_lua_fname}")
endmacro()
