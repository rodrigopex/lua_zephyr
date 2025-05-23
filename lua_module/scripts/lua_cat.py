import argparse

argparser = argparse.ArgumentParser(
    description="Convert Lua script files to C const strings."
)
argparser.add_argument("file", type=str, help="Lua script file to convert")
args = argparser.parse_args()

with open(args.file, "r") as f:
    script = f.read()
    strings = ["\\\n"]
    for line in script.splitlines():
        line = line.replace('"', '\\"')
        line = line.replace("\r", "")

        strings.append(line)

strings = " \\\n".join(strings)

print(strings, end="")
