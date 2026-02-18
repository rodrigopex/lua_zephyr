# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../")

include(${ZEPHYR_EXTRA_MODULES}/lua_kconfig.cmake)
lua_add_source_thread(src/hello_world.lua)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

include_directories(include/)

add_lua_file("src/sample01.lua")
lua_generate_threads()

file(GLOB_RECURSE SRC ${CMAKE_SOURCE_DIR} "src/*.c")

target_sources(app PRIVATE ${SRC})
