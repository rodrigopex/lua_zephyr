# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../lua_zephyr/")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(heavy_sample)

include_directories(include/)

add_lua_thread("src/heavy.lua")

file(GLOB_RECURSE SRC ${CMAKE_SOURCE_DIR} "src/*.c")

target_sources(app PRIVATE ${SRC})
