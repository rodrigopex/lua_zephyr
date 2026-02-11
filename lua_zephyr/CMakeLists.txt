if(CONFIG_LUA)
    set(LUA_DIR "${ZEPHYR_CURRENT_MODULE_DIR}/")
    set(INC_DIR "${LUA_DIR}/include/")
    set(SRC_DIR "${LUA_DIR}/src/")
    set(PARENT_SRC_DIR "${LUA_DIR}/../../zephyr")
    set(LUA_PARENT_DIR "${LUA_DIR}/../")

    message(WARNING "Lua ENABLED.")
    message(WARNING " *** Lua dir: ${LUA_DIR}")

    zephyr_include_directories("${PARENT_SRC_DIR}/include/"
                             "${INC_DIR}/"
    )

    zephyr_library()

    # The lua files
    file(GLOB SRC "${SRC_DIR}/*.c")

    if(CONFIG_LUA_PRECOMPILE_ONLY)
        list(REMOVE_ITEM SRC
            "${SRC_DIR}/lparser.c"
            "${SRC_DIR}/llex.c"
            "${SRC_DIR}/lcode.c"
        )
    endif()

    zephyr_library_sources("${SRC}"
                           "${SRC_DIR}/lua_zephyr/utils.c"
                           "${SRC_DIR}/lua_zephyr/lua_zbus.c"
                           "${SRC_DIR}/lua_zephyr/lua_msg_descr.c"
                           "${SRC_DIR}/lua_zephyr/lua_repl.c"
    )

    zephyr_library_sources_ifdef(CONFIG_LUA_PRECOMPILE_ONLY "${SRC_DIR}/lua_zephyr/lua_parser_stubs.c")
    zephyr_library_sources_ifdef(CONFIG_LUA_FS "${SRC_DIR}/lua_zephyr/lua_fs.c")
    zephyr_library_sources_ifdef(CONFIG_LUA_FS_SHELL "${SRC_DIR}/lua_zephyr/lua_fs_shell.c")

    # Add the helper functions to be used inside the user CMakeLists.txt. For
    # now, only `add_lua_file` exists.
    include("${LUA_DIR}/lua.cmake")
endif()
