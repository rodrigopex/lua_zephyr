local z = zephyr

z.printk("Hello from LittleFS!")

-- Load and execute greet.lua from the filesystem
dofile("greet.lua")
z.printk("dofile returned successfully")

-- List files on the filesystem
local files = fs.list()
for _, f in ipairs(files) do
    z.printk("  file: " .. f.name .. " (" .. f.size .. " bytes)")
end
