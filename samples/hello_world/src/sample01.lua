local z = require("zephyr")

local n = 0
repeat
	z.printk("-")
	z.msleep(1000)
	n = n + 1
until n == 5

z.printk("Sample 01, finished successfully")
