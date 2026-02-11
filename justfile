# ── Aliases ──────────────────────────────────────────────────────────────────────

alias b := build
alias bb := rebuild
alias c := clean
alias r := run
alias ds := debugserver
alias da := attach

# ── Variables ────────────────────────────────────────────────────────────────────
# Extra flags passed to west build (e.g. -DCONFIG_LUA_REPL=y)

flags := ""

# West build output directory

build_dir := './build'
c_build_dir := "-d " + build_dir

# Target board for west build

board := "qemu_cortex_m3"
c_board := "-b " + board

# Application source directory

project_dir := 'app'

# ── Build ────────────────────────────────────────────────────────────────────────

# Full configure + build
build:
    west build {{ c_build_dir }} {{ c_board }} {{ project_dir }} -- {{ flags }}

# Rebuild the project
rebuild:
    west build

# Clean build directory
clean:
    rm -rf {{ build_dir }}

# ── Run & Debug ──────────────────────────────────────────────────────────────────

# Clean, build, and run
minimal:
    just c b r

# Run the application in QEMU
run:
    west build -t run

# Open menuconfig
config:
    west build -d {{ build_dir }} -t menuconfig

# Start QEMU GDB server
debugserver:
    west build -t debugserver_qemu

# Attach GDB to a running debug session
attach:
    $ZEPHYR_SDK_INSTALL_DIR/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb -x {{ project_dir }}/gdbinit {{ build_dir }}/zephyr/zephyr.elf

# ── Testing ──────────────────────────────────────────────────────────────────────

# Run test suite via twister
test:
    rm -rf /tmp/lua_tests
    west twister -p mps2/an385 -T samples -O /tmp/lua_tests

# ── Code Quality ─────────────────────────────────────────────────────────────────

# Format project C/H files (excludes core Lua sources)
format:
    clang-format -i lua_zephyr/src/lua_zephyr/*.c lua_zephyr/include/lua_zephyr/*.h
    clang-format -i samples/*/src/*.c samples/*/include/*.h

# ── Default ──────────────────────────────────────────────────────────────────────

default:
    @just --list
