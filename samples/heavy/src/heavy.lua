local zephyr = require("zephyr")
local string = require("string")

for i = 1, 10 do
	local name = "func_" .. i
	_ENV[name] = function()
		local acc = 0
		local str = ""
		for j = 1, 50 do
			acc = acc + j * j - j
			str = str .. string.char(65 + (j % 26))
			if #str > 500 then
				str = string.sub(str, 200)
			end
		end

		local function recurse(n)
			if n <= 0 then
				return 1
			end
			return n + recurse(n - 1)
		end
		acc = acc + recurse(10)

		local t = {}
		for k = 1, 200 do
			t[k] = k * acc
		end
		local sum = 0
		for k = 1, #t do
			if t[k] % 2 == 0 then
				sum = sum + t[k]
			else
				sum = sum - t[k]
			end
		end

		return acc + sum + #str
	end
end

local results = {}
for i = 1, 10 do
	local name = "func_" .. i
	results[i] = _ENV[name]()
end

zephyr.printk("Heavy load sample finished")
