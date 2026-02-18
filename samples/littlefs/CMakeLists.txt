# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../")

include(${ZEPHYR_EXTRA_MODULES}/luaz.cmake)
luaz_define_fs_thread(/lfs/hello_fs.lua)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(littlefs_sample)

include_directories(include/)

# Embed Lua scripts as C strings for bootstrap (written to FS at boot)
luaz_add_file("src/hello_fs.lua")
luaz_add_file("src/greet.lua")

luaz_generate_threads()

file(GLOB_RECURSE SRC ${CMAKE_SOURCE_DIR} "src/*.c")

target_sources(app PRIVATE ${SRC})
