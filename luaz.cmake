# luaz.cmake — Pre-Zephyr Lua thread definition and Kconfig generation.
#
# Include this file BEFORE find_package(Zephyr) and use the luaz_define_*_thread()
# functions to define Lua threads. Each call appends to the corresponding list
# variable and generates a per-thread Kconfig fragment with STACK_SIZE,
# HEAP_SIZE, and PRIORITY options that default to the global values.
#
# After find_package(Zephyr), call luaz_generate_threads() to generate the
# thread C source files.
#
# Example:
#   include(${ZEPHYR_EXTRA_MODULES}/luaz.cmake)
#   luaz_define_bytecode_thread(src/producer.lua)
#   luaz_define_bytecode_thread(src/consumer.lua)
#   find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
#   project(my_app)
#   luaz_generate_threads()

set(_LUAZ_KCONFIG_DIR "${CMAKE_BINARY_DIR}/Kconfig")
file(MAKE_DIRECTORY "${_LUAZ_KCONFIG_DIR}")

macro(_luaz_write_thread_kconfig _name)
    string(TOUPPER "${_name}" _name_upper)
    set(_kconfig_file "${_LUAZ_KCONFIG_DIR}/Kconfig.lua_thread.${_name}")

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

# luaz_define_source_thread(FILE_NAME_PATH)
#
# Define a source-embedded Lua thread. Appends to LUAZ_SOURCE_THREADS
# and generates the per-thread Kconfig fragment.
macro(luaz_define_source_thread _path)
    list(APPEND LUAZ_SOURCE_THREADS "${_path}")
    set(_lua_thread_path "${_path}")
    cmake_path(GET _lua_thread_path FILENAME _lua_fname)
    cmake_path(REMOVE_EXTENSION _lua_fname OUTPUT_VARIABLE _lua_fname)
    _luaz_write_thread_kconfig("${_lua_fname}")
endmacro()

# luaz_define_bytecode_thread(FILE_NAME_PATH)
#
# Define a bytecode Lua thread. Appends to LUAZ_BYTECODE_THREADS
# and generates the per-thread Kconfig fragment.
macro(luaz_define_bytecode_thread _path)
    list(APPEND LUAZ_BYTECODE_THREADS "${_path}")
    set(_lua_thread_path "${_path}")
    cmake_path(GET _lua_thread_path FILENAME _lua_fname)
    cmake_path(REMOVE_EXTENSION _lua_fname OUTPUT_VARIABLE _lua_fname)
    _luaz_write_thread_kconfig("${_lua_fname}")
endmacro()

# luaz_define_fs_thread(SCRIPT_FS_PATH)
#
# Define an FS-backed Lua thread. Appends to LUAZ_FS_THREADS
# and generates the per-thread Kconfig fragment.
macro(luaz_define_fs_thread _path)
    list(APPEND LUAZ_FS_THREADS "${_path}")
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" _lua_fname "${_path}")
    string(REGEX REPLACE "^_+" "" _lua_fname "${_lua_fname}")
    _luaz_write_thread_kconfig("${_lua_fname}")
endmacro()
