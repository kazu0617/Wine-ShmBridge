#!/usr/bin/env python3
"""Keep Wine's valid current directory for Unix paths sent by the Linux host."""

from pathlib import Path


source = Path("work/Resonite.bundled-original.dll")
output = Path("work/Resonite.bundled-unixpathfix.dll")
data = bytearray(source.read_bytes())

# QueueLoop STARTRENDERER object initializer:
#   dup; ldloc.s 9; callvirt ProcessStartInfo::set_WorkingDirectory
# Wine accepts the Unix absolute filename, but Path.GetDirectoryName produces a
# backslash-rooted working directory (\home\...) that Win32 rejects with 267.
matches = []
for index in range(len(data) - 13):
    if (
        data[index : index + 3] == bytes.fromhex("25 11 09")
        and data[index + 3] == 0x6F
        and data[index + 8] == 0x28
    ):
        matches.append(index)

if len(matches) != 1:
    raise SystemExit(f"unexpected renderer initializer matches: {[hex(i) for i in matches]}")

index = matches[0]
data[index : index + 8] = b"\x00" * 8
output.write_bytes(data)
print(f"patched {source} -> {output} at {index:#x}")
