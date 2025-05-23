local z = zephyr
local zbus = zbus_wrappers

z.printk("Lua producer thread started...[ok]")

local x = 0

local getrandom = function(seed)
	local A1 = 71047
	local B1 = 8810
	local M1 = 7602

	return (A1 * seed + B1) % M1
end

while true do
	z.msleep(1000)
	zbus.pub_random_input(getrandom(x) % 100, 200)
	x = (x + 1) * 3
end
