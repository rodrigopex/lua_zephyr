module("zephyr")
---@meta
zephyr = {}

function zephyr.msleep(duration_ms) end
function zephyr.println(message) end
function zephyr.printk(message) end

zbus = {}
---
--- Pub to random input channel
---
---@param data integer # Random interger to publish.
---@param timeout integer # Publication timeout.
function zbus.chan_pub(chan, data, timeout) end
function zbus.sub_wait_msg(obs, chan, timeout) end

struct = {}
function struct.pack(fmt, data, ...) end
function struct.unpack(fmt, s) end
