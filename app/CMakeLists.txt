# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../lua_module/")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_lua)

include_directories(include/)

add_lua_file("src/sample01.lua")
add_lua_thread("src/hello_world.lua")

file(GLOB_RECURSE SRC ${CMAKE_SOURCE_DIR} "src/*.c")

target_sources(app PRIVATE ${SRC})
