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
