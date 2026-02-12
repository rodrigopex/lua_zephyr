if(CONFIG_LUA)
    set(LUA_DIR "${ZEPHYR_CURRENT_MODULE_DIR}/")
    set(LUA_CORE_DIR "${LUA_DIR}/lua/")
    set(INC_DIR "${LUA_DIR}/include/")
    set(SRC_DIR "${LUA_DIR}/src/")
    set(PARENT_SRC_DIR "${LUA_DIR}/../zephyr")

    message(WARNING "Lua ENABLED.")
    message(WARNING " *** Lua dir: ${LUA_DIR}")

    zephyr_include_directories("${PARENT_SRC_DIR}/include/"
                             "${INC_DIR}/"
                             "${LUA_CORE_DIR}/"
    )

    zephyr_compile_definitions(LUA_32BITS)

    zephyr_library()

    # Lua core sources from the git submodule
    file(GLOB SRC "${LUA_CORE_DIR}/l*.c")
    # Exclude standalone interpreter and internal test harness from the build
    list(REMOVE_ITEM SRC "${LUA_CORE_DIR}/lua.c" "${LUA_CORE_DIR}/ltests.c")

    if(CONFIG_LUA_PRECOMPILE_ONLY)
        list(REMOVE_ITEM SRC
            "${LUA_CORE_DIR}/lparser.c"
            "${LUA_CORE_DIR}/llex.c"
            "${LUA_CORE_DIR}/lcode.c"
        )
    endif()

    zephyr_library_sources("${SRC}"
                           "${SRC_DIR}/lua_zephyr/luaz_utils.c"
                           "${SRC_DIR}/lua_zephyr/luaz_zbus.c"
                           "${SRC_DIR}/lua_zephyr/luaz_msg_descr.c"
                           "${SRC_DIR}/lua_zephyr/luaz_repl.c"
    )

    zephyr_library_sources_ifdef(CONFIG_LUA_PRECOMPILE_ONLY "${SRC_DIR}/lua_zephyr/luaz_parser_stubs.c")
    zephyr_library_sources_ifdef(CONFIG_LUA_FS "${SRC_DIR}/lua_zephyr/luaz_fs.c")
    zephyr_library_sources_ifdef(CONFIG_LUA_FS_SHELL "${SRC_DIR}/lua_zephyr/luaz_fs_shell.c")

    # Add the helper functions to be used inside the user CMakeLists.txt. For
    # now, only `add_lua_file` exists.
    include("${LUA_DIR}/lua.cmake")
endif()
