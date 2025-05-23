---@meta
zephyr = {}

function zephyr.msleep(duration_ms) end
function zephyr.println(message) end
function zephyr.print(message) end

zbus = {}
---
--- Pub to random input channel
---
---@param data integer # Random interger to publish.
---@param timeout integer # Publication timeout.
function zbus.pub_random_input(data, timeout) end
