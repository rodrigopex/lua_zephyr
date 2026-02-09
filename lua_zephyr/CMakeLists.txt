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

    zephyr_library_sources("${SRC}"
                           "${SRC_DIR}/lua_zephyr/utils.c"
                           "${SRC_DIR}/lua_zephyr/lua_zbus.c"
                           "${SRC_DIR}/lua_zephyr/lua_msg_descr.c"
                           "${SRC_DIR}/lua_zephyr/lua_repl.c"
    )

    # Add the helper functions to be used inside the user CMakeLists.txt. For
    # now, only `add_lua_file` exists.
    include("${LUA_DIR}/lua.cmake")
endif()
