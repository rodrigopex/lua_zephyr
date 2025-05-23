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
