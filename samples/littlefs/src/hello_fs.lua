local z = require("zephyr")
local fs = z.fs

z.printk("Hello from LittleFS!")

z.printk("------------------------------------------")
-- Load and execute greet.lua from the filesystem
dofile("greet.lua")
z.printk("dofile returned successfully")

z.printk("------------------------------------------")

local lua_info_def, err = loadfile("info.lua")
if lua_info_def then
    lua_info_def()
    PrintLuaData()
else
    z.printk("info.lua not found: " .. err)
end

z.printk("------------------------------------------")

-- List files on the filesystem
local files = fs.list()
for _, f in ipairs(files) do
    z.printk("  file: " .. f.name .. " (" .. f.size .. " bytes)")
end

z.printk("------------------------------------------")
