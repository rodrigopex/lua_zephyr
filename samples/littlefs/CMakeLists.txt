# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(littlefs_sample)

include_directories(include/)

# Embed Lua scripts as C strings for bootstrap (written to FS at boot)
add_lua_file("src/hello_fs.lua")
add_lua_file("src/greet.lua")

# FS-backed thread: loads hello_fs.lua from LittleFS at runtime
add_lua_fs_thread("/lfs/hello_fs.lua")

file(GLOB_RECURSE SRC ${CMAKE_SOURCE_DIR} "src/*.c")

target_sources(app PRIVATE ${SRC})
