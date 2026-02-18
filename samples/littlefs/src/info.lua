local zephyr = require("zephyr")
local string = require("string")

function PrintLuaData()
    local data = {
        language = "Lua",
        version = _VERSION,
        type_system = "dynamically typed",
        paradigm = "multi-paradigm",
        designed_by = "Roberto Ierusalimschy",
        first_appeared = 1993,
        file_extension = ".lua",
    }

    zephyr.printk("Lua Language Data:")
    for key, value in pairs(data) do
        zephyr.printk(string.format("  %-16s: %s", key, tostring(value)))
    end
end
