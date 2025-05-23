alias b := build
alias bb := rebuild
alias c := clean
alias r := run
alias ds := debugserver
alias da := attach

# connect to wifi command shell: wifi connect -s NETUNO -k 1 -p 11311118
flags := ""

build_dir := './build'
c_build_dir := "-d " + build_dir

board := "qemu_cortex_m3"
c_board := "-b " + board

project_dir := 'app'

target_serial_port := '/dev/ttyACM0'

default:
    @just --list

minimal:
    just c b r

# Clean build directory
clean:
    rm -rf {{ build_dir }}

# Rebuild the project
rebuild:
    west build

build:
    west build {{ c_build_dir }} {{ c_board }} {{ project_dir }} -- {{ flags }}


run:
    west build -t run

# Open the menuconfig of the project using variables: build_dir
config:
    west build -d {{ build_dir }} -t menuconfig

debugserver:
    west build -t debugserver_qemu

attach:
    $ZEPHYR_SDK_INSTALL_DIR/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb -x {{project_dir}}/gdbinit {{ build_dir }}/zephyr/zephyr.elf

kb arg:
    @fend @no_trailing_newline "{{arg}} << 10" | wl-copy
