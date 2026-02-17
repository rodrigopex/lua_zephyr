--- Producer script: demonstrates zbus pub/sub with nested messages and
--- accelerometer data generation from a Lua thread.

local zephyr = require("zephyr")
local zbus = zephyr.zbus

local chan_version = zbus.channel_declare("chan_version")
local chan_sensor_config = zbus.channel_declare("chan_sensor_config")
local chan_acc_data = zbus.channel_declare("chan_acc_data")
local msub_acc_consumed = zbus.observer_declare("msub_acc_consumed")

--- Read and display the system version from the version channel.
local err, msg = chan_version:read(500)
if msg then
    zephyr.printk("System version: v" .. msg.major .. "." .. msg.minor .. "." .. msg.patch .. "_" .. msg.hardware_id)
end

--- Publish a sensor configuration with a nested offset struct, then read it
--- back to verify the round-trip through the descriptor-based serialization.
local sensor_cfg = { sensor_id = 42, offset = { x = 1, y = 2, z = 3 } }
err = chan_sensor_config:pub(sensor_cfg, 200)
if err == 0 then
    err, msg = chan_sensor_config:read(200)
    if msg then
        zephyr.printk("Sensor_config={sensor_id=" .. msg.sensor_id
            .. " offset={x=" .. msg.offset.x
            .. ", y=" .. msg.offset.y
            .. ", z=" .. msg.offset.z .. "}}")
    end
end

--- Linear congruential pseudo-random number generator.
--- @param seed number  Input seed value.
--- @return number      Pseudo-random value in [0, 7601].
local getrandom = function(seed)
    local A1 = 71047
    local B1 = 8810
    local M1 = 7602
    return (A1 * seed + B1) % M1
end

--- Accelerometer data template (mass and xyz components).
local acc_data = { x = 0, y = 0, z = 0 }

local i = 1

--- Main producer loop: generate random accelerometer samples, publish them on
--- chan_acc_data, and wait for a consumer acknowledgement on msub_acc_consumed.
--- The consumer's ack carries a count that drives the loop index forward.
while i < 10 do
    acc_data.x = getrandom(i * 0x6782) % 100
    acc_data.y = getrandom(i * 0x82716) % 100
    acc_data.z = getrandom(i * 0x9283) % 100

    zephyr.printk("<-- Lua producing data ")

    err = chan_acc_data:pub(acc_data, 200)
    if err ~= 0 then
        zephyr.printk("Could not pub to channel")
    end

    --- Wait for the consumer to process the data and send back an ack message.
    --- (number, number, ack_data_consumed)
    err, _, msg = msub_acc_consumed:wait_msg(250)

    if err ~= 0 then
        zephyr.printk("error: " .. err)
        break
    else
        if msg and msg.count then
            zephyr.printk("--> Lua received ack " .. msg.count)
            i = msg.count
        end
    end

    zephyr.msleep(200)
end
