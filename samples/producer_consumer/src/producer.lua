if require then
	require("zephyr")
end

local err, msg = zbus.chan_version:read(500)
if msg then
	zephyr.printk("System version: v" .. msg.major .. "." .. msg.minor .. "." .. msg.patch .. "_" .. msg.hardware_id)
end

local getrandom = function(seed)
	local A1 = 71047
	local B1 = 8810
	local M1 = 7602
	return (A1 * seed + B1) % M1
end

local acc_data = { x = 0, y = 0, z = 0, source = "lua" }

local i = 1

while i < 10 do
	acc_data.x = getrandom(i * 0x6782) % 100
	acc_data.y = getrandom(i * 0x82716) % 100
	acc_data.z = getrandom(i * 0x9283) % 100

	zephyr.printk("<-- Lua producing data ")

	err = zbus.chan_acc_data:pub(200, acc_data)
	if err ~= 0 then
		zephyr.printk("Could not pub to channel")
	end

	local data_array = {
		acc_data.x,
		acc_data.y,
		acc_data.z,
	}

	local acc_data_array = { data = data_array }

	err = zbus.chan_acc_data_array:pub(200, acc_data_array)
	if err ~= 0 then
		zephyr.printk("Could not pub to channel")
	end

	err, _, msg = zbus.msub_acc_consumed:wait_msg(250)

	if err ~= 0 then
		zephyr.printk("error: " .. err)
		break
	else
		if msg.count then
			zephyr.printk("--> Lua received ack " .. msg.count)
			i = msg.count
		end
	end

	zephyr.msleep(200)
end
