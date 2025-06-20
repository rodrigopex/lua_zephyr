# Overview
This is an initiative to try making Lua a module on Zephyr. There have been several attempts to achieve this in the past, but it remains elusive. This effort aims to make Lua an alternative for implementing application logic in embedded projects that rely on Zephyr RTOS. 

# The mission

- Be simple to use: ```lua_add_thread("src/some_script.lua")```
- Be flexible (giving the users ways to implement low-level things)
- Has a minimum bindings set (sleeps, logs, and zbus)
- Be as performant as possible

# Overview of the idea

We are assuming a project pattern where Lua scripts can only interact with C code via zbus channels and available kernel APIs. Every Lua file will run in its dedicated thread context and dedicated heap. 

![image](https://github.com/user-attachments/assets/c6bc2fc3-6ba3-45a1-98c0-98e4dd28f1bf)

