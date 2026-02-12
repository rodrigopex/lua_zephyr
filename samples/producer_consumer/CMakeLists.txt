# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(producer_consumer)

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)
set(NANOPB_OPTIONS
    "--nanopb_opt=-X"
    "--nanopb_opt=--custom-style=${ZEPHYR_LUA_MODULE_DIR}/scripts/nanopb_custom_naming.py.NamingStyleLuaC"
)
zephyr_nanopb_sources(app proto/channels.proto)

include_directories(include/)

add_lua_bytecode_thread("src/producer.lua")

file(GLOB_RECURSE SRC ${CMAKE_SOURCE_DIR} "src/*.c")

target_sources(app PRIVATE ${SRC})
