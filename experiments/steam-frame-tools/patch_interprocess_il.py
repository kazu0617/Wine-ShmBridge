#!/usr/bin/env python3
"""Patch bundled Interprocess IL to use Windows semaphores under Wine."""

from pathlib import Path


source = Path("work/Interprocess.bundled-original.dll")
output = Path("work/Interprocess.bundled-arm64fix.dll")
data = bytearray(source.read_bytes())

# Shared body for CreateWaiter/CreateReleaser. Replace:
#   ldarg.0; ldc.i4.0; newobj SemaphoreWine::.ctor(string,bool); ret
# with:
#   ldarg.0; newobj SemaphoreWindows::.ctor(string); ret; nop
old = bytes.fromhex("02 16 73 59 00 00 06 2a")
new = bytes.fromhex("02 73 68 00 00 06 2a 00")
matches = [index for index in range(len(data)) if data.startswith(old, index)]
if matches != [0xD12]:
    raise SystemExit(f"unexpected patch locations: {[hex(index) for index in matches]}")

data[matches[0] : matches[0] + len(old)] = new
output.write_bytes(data)
print(f"patched {source} -> {output} at {matches[0]:#x}")
