zephyr.printk("Lua producer thread started")

local my_object = { saldo = 1050.56, cliente = "Rodrigo" }

for k, v in pairs(my_object) do
	zephyr.printk(k .. ": " .. v)
end

zephyr.printk(tostring(zbus.chan_acc_data))

local getrandom = function(seed)
	local A1 = 71047
	local B1 = 8810
	local M1 = 7602
	return (A1 * seed + B1) % M1
end

local i = 1
local err
local ack_msg

while i < 10 do
	local msg =
		struct.pack("iii", getrandom(i * 0x6782) % 100, getrandom(i * 0x82716) % 100, getrandom(i * 0x9283) % 100)

	zephyr.printk("<-- Lua producing data")
	err = zbus.pub(zbus.chan_acc_data, msg, 200)
	if err ~= 0 then
		zephyr.printk("Could not pub to channel")
	end

	local r, ack_msg = zbus.sub_wait_msg(zbus.msub_acc_consumed, zbus.chan_acc_data_consumed, 200)

	if err ~= 0 then
		zephyr.printk("error: " .. err)
		break
	else
		local count = struct.unpack("i", ack_msg)
		if count then
			zephyr.printk("--> Lua receiving ack")
			i = count
		end
	end
	zephyr.msleep(200)
end
