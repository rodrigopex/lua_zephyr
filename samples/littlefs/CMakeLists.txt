# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../")

include(${ZEPHYR_EXTRA_MODULES}/lua_kconfig.cmake)
lua_add_fs_thread(/lfs/hello_fs.lua)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(littlefs_sample)

include_directories(include/)

# Embed Lua scripts as C strings for bootstrap (written to FS at boot)
add_lua_file("src/hello_fs.lua")
add_lua_file("src/greet.lua")

lua_generate_threads()

file(GLOB_RECURSE SRC ${CMAKE_SOURCE_DIR} "src/*.c")

target_sources(app PRIVATE ${SRC})
