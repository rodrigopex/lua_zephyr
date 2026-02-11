"""Compile a Lua script to bytecode and emit a C-embeddable byte array.

Runs the host-built luac with -s (strip debug info) to produce a
bytecode file, then prints two lines to stdout:
  line 1: byte count
  line 2: comma-separated hex bytes (e.g. 0x1b, 0x4c, ...)

Used by lua.cmake's add_lua_bytecode_file() and add_lua_bytecode_thread()
via CMake's execute_process.
"""

import argparse
import subprocess
import tempfile
import os
import sys

argparser = argparse.ArgumentParser(
    description="Compile Lua scripts to bytecode and emit C byte arrays."
)
argparser.add_argument("luac", type=str, help="Path to the host luac binary")
argparser.add_argument("file", type=str, help="Lua script file to compile")
args = argparser.parse_args()

with tempfile.NamedTemporaryFile(suffix=".luac", delete=False) as tmp:
    tmp_path = tmp.name

try:
    result = subprocess.run(
        [args.luac, "-s", "-o", tmp_path, args.file],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print(f"luac error: {result.stderr}", file=sys.stderr)
        sys.exit(1)

    with open(tmp_path, "rb") as f:
        data = f.read()

    print(len(data))
    print(", ".join(f"0x{b:02x}" for b in data))
finally:
    os.unlink(tmp_path)
