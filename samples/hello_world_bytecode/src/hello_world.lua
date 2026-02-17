local z = require("zephyr")

z.printk("Hello from lua thread")
z.log_err("This is an error log")
z.log_dbg("This is a debug log")
z.log_inf("This is an info log")
z.log_wrn("This is a warning log")

z.msleep(3000)

z.printk("bye")
