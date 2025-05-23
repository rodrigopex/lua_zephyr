zephyr.printk("Lua producer thread started")

local getrandom = function(seed)
	local A1 = 71047
	local B1 = 8810
	local M1 = 7602
	return (A1 * seed + B1) % M1
end

for i = 1, 10 do
	zephyr.msleep(1000)
	local err = zbus.pub_random_input(getrandom(i * 0xFFFFab) % 100, 200)
	if err ~= 0 then
		zephyr.printk("Could not pub to channel")
	end
end
