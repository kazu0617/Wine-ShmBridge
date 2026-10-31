#!/usr/bin/env python3
"""Bind the official ARM64 .NET apphost template to Renderite.Host.dll."""

from pathlib import Path


source = next(Path("work/apphost-win-arm64").rglob("apphost.exe"))
output = Path("work/Resonite.arm64-apphost.exe")
data = bytearray(source.read_bytes())
placeholder = b"c3ab8ff13720e8ad9047dd39466b3c8974e592c2fa383d4a3960714caef0c4f2"
target = b"Renderite.Host.dll"
offset = data.find(placeholder)
if offset < 0 or data.find(placeholder, offset + 1) >= 0:
    raise SystemExit("apphost placeholder missing or ambiguous")
data[offset : offset + len(placeholder)] = target + bytes(len(placeholder) - len(target))
output.write_bytes(data)
print(f"bound {source} -> {output} at {offset:#x}")
