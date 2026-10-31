#!/usr/bin/env python3
"""Make Renderite's Wine semaphore factory use its Windows implementation."""

from pathlib import Path


source = Path("work/Interprocess.renderer-original.dll")
output = Path("work/Interprocess.renderer-windowssemaphore.dll")
data = bytearray(source.read_bytes())

# CreateWaiter/CreateReleaser Wine branch:
#   ldarg.0; ldc.i4.0; newobj SemaphoreWine::.ctor(string,bool); ret
# becomes:
#   ldarg.0; newobj SemaphoreWindows::.ctor(string); ret; nop
old = bytes.fromhex("02 16 73 be 00 00 06 2a")
new = bytes.fromhex("02 73 ca 00 00 06 2a 00")
matches = [index for index in range(len(data)) if data.startswith(old, index)]
if matches != [0x13E0, 0x141F]:
    raise SystemExit(f"unexpected patch locations: {[hex(index) for index in matches]}")

for index in matches:
    data[index : index + len(old)] = new

output.write_bytes(data)
print(f"patched {source} -> {output} at {', '.join(hex(i) for i in matches)}")
