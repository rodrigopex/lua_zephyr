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
        file(GLOB LUAC_HOST_SRCS "${LUA_MOD_DIR}/src/l*.c")
        list(APPEND LUAC_HOST_SRCS "${LUA_MOD_DIR}/host_tools/luac.c")

        message(STATUS "Building host luac: ${LUAC_HOST}")
        execute_process(
            COMMAND ${HOST_CC}
                ${LUAC_HOST_SRCS}
                -I${LUA_MOD_DIR}/include
                -O2 -DLUA_USE_POSIX -lm
                -o ${LUAC_HOST}
            RESULT_VARIABLE LUAC_BUILD_RESULT
            ERROR_VARIABLE LUAC_BUILD_ERROR
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()
endif()

# add_lua_file(FILE_NAME_PATH)
#
# Embed a .lua script as a C const string header.
#
# Runs lua_cat.py to convert the Lua source into a C-escaped string,
# then configures lua_template.h.in to produce <name>_lua_script.h.
# The generated header is placed under ${CMAKE_CURRENT_BINARY_DIR}/lua
# and made available via include_directories.
#
# Arguments:
#   FILE_NAME_PATH - Path to the .lua file (relative to project source dir).
function(add_lua_file FILE_NAME_PATH)
    cmake_path(GET FILE_NAME_PATH FILENAME FILE_NAME)
    cmake_path(REMOVE_EXTENSION FILE_NAME OUTPUT_VARIABLE FILE_NAME)

    execute_process(COMMAND ${PYTHON_EXECUTABLE}
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/lua_cat.py" ${FILE_NAME_PATH}
    OUTPUT_VARIABLE LUA_CONTENT WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
  )

    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/lua_template.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lua/${FILE_NAME}_lua_script.h")

    include_directories("${CMAKE_BINARY_DIR}/lua")
endfunction()


# add_lua_thread(FILE_NAME_PATH)
#
# Generate a complete Lua thread (Zephyr thread + embedded script).
#
# Runs lua_cat.py to convert the Lua source, then configures
# lua_thread.c.in to produce <name>_lua_thread.c containing:
#   - A dedicated sys_heap and stack
#   - A weak _lua_setup hook for pre-script initialization
#   - A K_THREAD_DEFINE that runs the embedded Lua script
#
# The generated .c file is automatically added to the `app` target.
#
# Arguments:
#   FILE_NAME_PATH - Path to the .lua file (relative to project source dir).
function(add_lua_thread FILE_NAME_PATH)
    cmake_path(GET FILE_NAME_PATH FILENAME FILE_NAME)
    cmake_path(REMOVE_EXTENSION FILE_NAME OUTPUT_VARIABLE FILE_NAME)

    execute_process(COMMAND ${PYTHON_EXECUTABLE}
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/lua_cat.py" ${FILE_NAME_PATH}
    OUTPUT_VARIABLE LUA_CONTENT WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
  )

    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/lua_thread.c.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lua/${FILE_NAME}_lua_thread.c")

    include_directories("${CMAKE_BINARY_DIR}/lua")

    target_sources(app PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/lua/${FILE_NAME}_lua_thread.c"
    )
endfunction()


# add_lua_bytecode_file(FILE_NAME_PATH)
#
# Embed a pre-compiled Lua bytecode as a C uint8_t array header.
#
# Runs lua_compile.py (which invokes the host-built luac -s) to produce
# stripped bytecode, then configures lua_bytecode_template.h.in to produce
# <name>_lua_bytecode.h.  The generated header is placed under
# ${CMAKE_CURRENT_BINARY_DIR}/lua and made available via include_directories.
#
# Requires CONFIG_LUA_PRECOMPILE=y.
#
# Arguments:
#   FILE_NAME_PATH - Path to the .lua file (relative to project source dir).
function(add_lua_bytecode_file FILE_NAME_PATH)
    cmake_path(GET FILE_NAME_PATH FILENAME FILE_NAME)
    cmake_path(REMOVE_EXTENSION FILE_NAME OUTPUT_VARIABLE FILE_NAME)

    execute_process(COMMAND ${PYTHON_EXECUTABLE}
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/lua_compile.py"
    "${LUAC_HOST}" ${FILE_NAME_PATH}
    OUTPUT_VARIABLE LUA_COMPILE_OUTPUT WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
  )

    # Parse output: line 1 = byte count, line 2 = hex bytes
    string(REGEX MATCH "^([0-9]+)\n(.+)$" _match "${LUA_COMPILE_OUTPUT}")
    set(LUA_BYTECODE_LEN "${CMAKE_MATCH_1}")
    set(LUA_BYTECODE "${CMAKE_MATCH_2}")

    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/lua_bytecode_template.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lua/${FILE_NAME}_lua_bytecode.h")

    include_directories("${CMAKE_BINARY_DIR}/lua")
endfunction()


# add_lua_bytecode_thread(FILE_NAME_PATH)
#
# Generate a complete Lua thread loading pre-compiled bytecode.
#
# Runs lua_compile.py to produce stripped bytecode, then configures
# lua_bytecode_thread.c.in to produce <name>_lua_bytecode_thread.c
# containing:
#   - A dedicated sys_heap and stack
#   - A weak _lua_setup hook for pre-script initialization
#   - A K_THREAD_DEFINE that loads and runs the bytecode via luaL_loadbuffer
#
# The generated .c file is automatically added to the `app` target.
#
# Requires CONFIG_LUA_PRECOMPILE=y.
#
# Arguments:
#   FILE_NAME_PATH - Path to the .lua file (relative to project source dir).
function(add_lua_bytecode_thread FILE_NAME_PATH)
    cmake_path(GET FILE_NAME_PATH FILENAME FILE_NAME)
    cmake_path(REMOVE_EXTENSION FILE_NAME OUTPUT_VARIABLE FILE_NAME)

    execute_process(COMMAND ${PYTHON_EXECUTABLE}
    "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/lua_compile.py"
    "${LUAC_HOST}" ${FILE_NAME_PATH}
    OUTPUT_VARIABLE LUA_COMPILE_OUTPUT WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
  )

    # Parse output: line 1 = byte count, line 2 = hex bytes
    string(REGEX MATCH "^([0-9]+)\n(.+)$" _match "${LUA_COMPILE_OUTPUT}")
    set(LUA_BYTECODE_LEN "${CMAKE_MATCH_1}")
    set(LUA_BYTECODE "${CMAKE_MATCH_2}")

    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/lua_bytecode_thread.c.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lua/${FILE_NAME}_lua_bytecode_thread.c")

    include_directories("${CMAKE_BINARY_DIR}/lua")

    target_sources(app PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/lua/${FILE_NAME}_lua_bytecode_thread.c"
    )
endfunction()
