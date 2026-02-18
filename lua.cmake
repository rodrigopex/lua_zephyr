# Build host luac at configure time (when CONFIG_LUA_PRECOMPILE is enabled).
# This compiles all Lua core sources + luac.c for the host platform, producing
# a native luac binary used to pre-compile .lua scripts to bytecode.
if(CONFIG_LUA_PRECOMPILE)
    set(LUA_MOD_DIR "${CMAKE_CURRENT_LIST_DIR}")
    find_program(HOST_CC NAMES cc gcc REQUIRED)

    set(LUAC_HOST "${CMAKE_CURRENT_BINARY_DIR}/host_tools/luac" CACHE INTERNAL
        "Path to host-built luac binary for Lua bytecode pre-compilation")

    if(NOT EXISTS "${LUAC_HOST}")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/host_tools")
        file(GLOB LUAC_HOST_SRCS "${LUA_MOD_DIR}/lua/l*.c")
        list(REMOVE_ITEM LUAC_HOST_SRCS "${LUA_MOD_DIR}/lua/lua.c" "${LUA_MOD_DIR}/lua/ltests.c")
        list(APPEND LUAC_HOST_SRCS "${LUA_MOD_DIR}/host_tools/luac.c")

        message(STATUS "Building host luac: ${LUAC_HOST}")
        execute_process(
            COMMAND ${HOST_CC}
                ${LUAC_HOST_SRCS}
                -I${LUA_MOD_DIR}/include
                -I${LUA_MOD_DIR}/lua
                -O2 -DLUA_USE_POSIX -DLUA_32BITS -lm
                -o ${LUAC_HOST}
            RESULT_VARIABLE LUAC_BUILD_RESULT
            ERROR_VARIABLE LUAC_BUILD_ERROR
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()
endif()

# Path to the unified Lua generator script used by all add_lua_* functions.
set(LUA_GENERATE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/scripts/luaz_gen.py"
    CACHE INTERNAL "Path to luaz_gen.py")

# _lua_generate_thread_kconfig(FILE_NAME)
#
# Generate a per-thread Kconfig fragment with STACK_SIZE, HEAP_SIZE,
# and PRIORITY options that default to the global CONFIG_LUA_THREAD_* values.
#
# The file is written to ${KCONFIG_BINARY_DIR}/Kconfig.lua_thread.${FILE_NAME}
# and picked up by osource in Kconfig.luaz.
function(_lua_generate_thread_kconfig FILE_NAME)
    string(TOUPPER "${FILE_NAME}" FILE_NAME_UPPER)
    set(KCONFIG_FILE "${KCONFIG_BINARY_DIR}/Kconfig.lua_thread.${FILE_NAME}")

    if(EXISTS "${KCONFIG_FILE}")
        return()
    endif()

    file(WRITE "${KCONFIG_FILE}"
"config ${FILE_NAME_UPPER}_LUA_THREAD_STACK_SIZE\n\
    int \"${FILE_NAME} Lua thread stack size\"\n\
    default LUA_THREAD_STACK_SIZE\n\
    help\n\
      Stack size for the ${FILE_NAME} Lua thread.\n\
\n\
config ${FILE_NAME_UPPER}_LUA_THREAD_HEAP_SIZE\n\
    int \"${FILE_NAME} Lua thread heap size\"\n\
    default LUA_THREAD_HEAP_SIZE\n\
    help\n\
      Heap size for the ${FILE_NAME} Lua thread.\n\
\n\
config ${FILE_NAME_UPPER}_LUA_THREAD_PRIORITY\n\
    int \"${FILE_NAME} Lua thread priority\"\n\
    default LUA_THREAD_PRIORITY\n\
    help\n\
      Priority for the ${FILE_NAME} Lua thread.\n"
    )

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${KCONFIG_FILE}")
endfunction()


# add_lua_file(FILE_NAME_PATH)
#
# Embed a .lua script as a C const string header.
#
# Runs lua_generate.py in source mode to produce <name>_lua_script.h.
# The generated header is placed under ${CMAKE_CURRENT_BINARY_DIR}/lua
# and made available via include_directories.
#
# The .lua source file is tracked as a dependency so that modifying it
# triggers regeneration on incremental builds.
#
# Arguments:
#   FILE_NAME_PATH - Path to the .lua file (relative to project source dir).
function(add_lua_file FILE_NAME_PATH)
    cmake_path(GET FILE_NAME_PATH FILENAME FILE_NAME)
    cmake_path(REMOVE_EXTENSION FILE_NAME OUTPUT_VARIABLE FILE_NAME)

    set(LUA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME_PATH}")
    set(LUA_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/lua")
    set(LUA_OUTPUT "${LUA_OUTPUT_DIR}/${FILE_NAME}_lua_script.h")
    set(LUA_TEMPLATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/templates/lua_template.h.in")

    add_custom_command(
        OUTPUT "${LUA_OUTPUT}"
        COMMAND ${PYTHON_EXECUTABLE} "${LUA_GENERATE_SCRIPT}"
            --mode source
            --template "${LUA_TEMPLATE}"
            --output "${LUA_OUTPUT}"
            --name "${FILE_NAME}"
            "${LUA_FILE}"
        DEPENDS "${LUA_FILE}" "${LUA_TEMPLATE}"
        COMMENT "Generating ${FILE_NAME}_lua_script.h from ${FILE_NAME}.lua"
    )

    add_custom_target(${FILE_NAME}_lua_header DEPENDS "${LUA_OUTPUT}")
    add_dependencies(app ${FILE_NAME}_lua_header)

    include_directories("${LUA_OUTPUT_DIR}")
endfunction()


# add_lua_thread(FILE_NAME_PATH)
#
# Generate a complete Lua thread (Zephyr thread + embedded script).
#
# Runs lua_generate.py in source mode to produce <name>_lua_thread.c
# containing:
#   - A dedicated sys_heap and stack
#   - A weak _lua_setup hook for pre-script initialization
#   - A K_THREAD_DEFINE that runs the embedded Lua script
#
# The generated .c file is automatically added to the `app` target.
# The .lua source file is tracked as a dependency for incremental builds.
#
# Arguments:
#   FILE_NAME_PATH - Path to the .lua file (relative to project source dir).
function(add_lua_thread FILE_NAME_PATH)
    cmake_path(GET FILE_NAME_PATH FILENAME FILE_NAME)
    cmake_path(REMOVE_EXTENSION FILE_NAME OUTPUT_VARIABLE FILE_NAME)

    set(LUA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME_PATH}")
    set(LUA_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/lua")
    set(LUA_OUTPUT "${LUA_OUTPUT_DIR}/${FILE_NAME}_lua_thread.c")
    set(LUA_TEMPLATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/templates/lua_thread.c.in")

    add_custom_command(
        OUTPUT "${LUA_OUTPUT}"
        COMMAND ${PYTHON_EXECUTABLE} "${LUA_GENERATE_SCRIPT}"
            --mode source
            --template "${LUA_TEMPLATE}"
            --output "${LUA_OUTPUT}"
            --name "${FILE_NAME}"
            "${LUA_FILE}"
        DEPENDS "${LUA_FILE}" "${LUA_TEMPLATE}"
        COMMENT "Generating ${FILE_NAME}_lua_thread.c from ${FILE_NAME}.lua"
    )

    _lua_generate_thread_kconfig("${FILE_NAME}")

    include_directories("${LUA_OUTPUT_DIR}")

    target_sources(app PRIVATE "${LUA_OUTPUT}")
endfunction()


# add_lua_bytecode_file(FILE_NAME_PATH)
#
# Embed a pre-compiled Lua bytecode as a C uint8_t array header.
#
# Runs lua_generate.py in bytecode mode (which invokes luac -s) to produce
# <name>_lua_bytecode.h.  The generated header is placed under
# ${CMAKE_CURRENT_BINARY_DIR}/lua and made available via include_directories.
#
# The .lua source file is tracked as a dependency for incremental builds.
#
# Requires CONFIG_LUA_PRECOMPILE=y.
#
# Arguments:
#   FILE_NAME_PATH - Path to the .lua file (relative to project source dir).
function(add_lua_bytecode_file FILE_NAME_PATH)
    cmake_path(GET FILE_NAME_PATH FILENAME FILE_NAME)
    cmake_path(REMOVE_EXTENSION FILE_NAME OUTPUT_VARIABLE FILE_NAME)

    set(LUA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME_PATH}")
    set(LUA_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/lua")
    set(LUA_OUTPUT "${LUA_OUTPUT_DIR}/${FILE_NAME}_lua_bytecode.h")
    set(LUA_TEMPLATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/templates/lua_bytecode_template.h.in")

    add_custom_command(
        OUTPUT "${LUA_OUTPUT}"
        COMMAND ${PYTHON_EXECUTABLE} "${LUA_GENERATE_SCRIPT}"
            --mode bytecode
            --template "${LUA_TEMPLATE}"
            --output "${LUA_OUTPUT}"
            --name "${FILE_NAME}"
            --luac "${LUAC_HOST}"
            "${LUA_FILE}"
        DEPENDS "${LUA_FILE}" "${LUA_TEMPLATE}"
        COMMENT "Generating ${FILE_NAME}_lua_bytecode.h from ${FILE_NAME}.lua"
    )

    add_custom_target(${FILE_NAME}_lua_bytecode_header DEPENDS "${LUA_OUTPUT}")
    add_dependencies(app ${FILE_NAME}_lua_bytecode_header)

    include_directories("${LUA_OUTPUT_DIR}")
endfunction()


# add_lua_bytecode_thread(FILE_NAME_PATH)
#
# Generate a complete Lua thread loading pre-compiled bytecode.
#
# Runs lua_generate.py in bytecode mode to produce
# <name>_lua_bytecode_thread.c containing:
#   - A dedicated sys_heap and stack
#   - A weak _lua_setup hook for pre-script initialization
#   - A K_THREAD_DEFINE that loads and runs the bytecode via luaL_loadbuffer
#
# The generated .c file is automatically added to the `app` target.
# The .lua source file is tracked as a dependency for incremental builds.
#
# Requires CONFIG_LUA_PRECOMPILE=y.
#
# Arguments:
#   FILE_NAME_PATH - Path to the .lua file (relative to project source dir).
function(add_lua_bytecode_thread FILE_NAME_PATH)
    cmake_path(GET FILE_NAME_PATH FILENAME FILE_NAME)
    cmake_path(REMOVE_EXTENSION FILE_NAME OUTPUT_VARIABLE FILE_NAME)

    set(LUA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME_PATH}")
    set(LUA_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/lua")
    set(LUA_OUTPUT "${LUA_OUTPUT_DIR}/${FILE_NAME}_lua_bytecode_thread.c")
    set(LUA_TEMPLATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/templates/lua_bytecode_thread.c.in")

    add_custom_command(
        OUTPUT "${LUA_OUTPUT}"
        COMMAND ${PYTHON_EXECUTABLE} "${LUA_GENERATE_SCRIPT}"
            --mode bytecode
            --template "${LUA_TEMPLATE}"
            --output "${LUA_OUTPUT}"
            --name "${FILE_NAME}"
            --luac "${LUAC_HOST}"
            "${LUA_FILE}"
        DEPENDS "${LUA_FILE}" "${LUA_TEMPLATE}"
        COMMENT "Generating ${FILE_NAME}_lua_bytecode_thread.c from ${FILE_NAME}.lua"
    )

    _lua_generate_thread_kconfig("${FILE_NAME}")

    include_directories("${LUA_OUTPUT_DIR}")

    target_sources(app PRIVATE "${LUA_OUTPUT}")
endfunction()


# add_lua_fs_thread(SCRIPT_FS_PATH)
#
# Generate a Lua thread that loads its script from the filesystem at runtime.
#
# Configures lua_fs_thread.c.in with the given FS path, producing a Zephyr
# thread that calls lua_fs_dofile() instead of embedding the script.
# The generated .c file is automatically added to the `app` target.
#
# Requires CONFIG_LUA_FS=y.
#
# Arguments:
#   SCRIPT_FS_PATH - Filesystem path the thread will load at runtime
#                    (e.g. "/lfs/hello_fs.lua" or just "hello_fs.lua").
function(add_lua_fs_thread SCRIPT_FS_PATH)
    # Derive a C-safe identifier from the path
    string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" FILE_NAME "${SCRIPT_FS_PATH}")
    string(REGEX REPLACE "^_+" "" FILE_NAME "${FILE_NAME}")

    set(LUA_FS_PATH "${SCRIPT_FS_PATH}")
    string(TOUPPER "${FILE_NAME}" FILE_NAME_UPPER)

    _lua_generate_thread_kconfig("${FILE_NAME}")

    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/templates/lua_fs_thread.c.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lua/${FILE_NAME}_lua_fs_thread.c")

    include_directories("${CMAKE_BINARY_DIR}/lua")

    target_sources(app PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/lua/${FILE_NAME}_lua_fs_thread.c"
    )
endfunction()


# add_lua_fs_file(SOURCE_PATH [FS_NAME])
#
# Register a Lua source file for embedding and later writing to the FS.
#
# Embeds the file as a C string header (via add_lua_file) so that the
# application can write it to the filesystem at boot using lua_fs_write_file().
#
# Arguments:
#   SOURCE_PATH - Path to the .lua file (relative to project source dir).
#   FS_NAME     - Optional: basename on the filesystem (defaults to source basename).
function(add_lua_fs_file SOURCE_PATH)
    add_lua_file("${SOURCE_PATH}")
endfunction()


# lua_generate_threads()
#
# Generate Lua threads from the LUA_SOURCE_THREADS, LUA_BYTECODE_THREADS,
# and LUA_FS_THREADS list variables. Each entry is processed by the
# corresponding add_lua_*_thread() function.
function(lua_generate_threads)
    foreach(_path ${LUA_SOURCE_THREADS})
        add_lua_thread("${_path}")
    endforeach()
    foreach(_path ${LUA_BYTECODE_THREADS})
        add_lua_bytecode_thread("${_path}")
    endforeach()
    foreach(_path ${LUA_FS_THREADS})
        add_lua_fs_thread("${_path}")
    endforeach()
endfunction()
