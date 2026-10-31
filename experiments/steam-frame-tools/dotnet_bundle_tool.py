#!/usr/bin/env python3
"""Inspect or replace an entry in a .NET single-file bundle."""

from __future__ import annotations

import argparse
import struct
import zlib
from pathlib import Path


SIGNATURE = bytes.fromhex(
    "8b1202b96a612038727b930214d7a03213f5b9e6efae3318ee3b2dce24b36aae"
)


def read_7bit(data: bytes | bytearray, pos: int) -> tuple[int, int]:
    value = 0
    shift = 0
    while True:
        byte = data[pos]
        pos += 1
        value |= (byte & 0x7F) << shift
        if not byte & 0x80:
            return value, pos
        shift += 7


def read_string(data: bytes | bytearray, pos: int) -> tuple[str, int]:
    length, pos = read_7bit(data, pos)
    return bytes(data[pos : pos + length]).decode("utf-8"), pos + length


def entries(data: bytes | bytearray) -> tuple[int, list[dict[str, int | str]]]:
    signature_pos = data.find(SIGNATURE)
    if signature_pos < 8:
        raise ValueError(".NET bundle signature not found")
    header_pos = struct.unpack_from("<q", data, signature_pos - 8)[0]
    pos = header_pos
    major, minor, count = struct.unpack_from("<IIi", data, pos)
    pos += 12
    bundle_id, pos = read_string(data, pos)
    if major >= 2:
        pos += 40
    result: list[dict[str, int | str]] = []
    for _ in range(count):
        metadata_pos = pos
        offset, size = struct.unpack_from("<qq", data, pos)
        pos += 16
        compressed_size = 0
        if major >= 6:
            compressed_size = struct.unpack_from("<q", data, pos)[0]
            pos += 8
        file_type = data[pos]
        pos += 1
        name, pos = read_string(data, pos)
        result.append(
            {
                "name": name,
                "offset": offset,
                "size": size,
                "compressed_size": compressed_size,
                "type": file_type,
                "metadata_pos": metadata_pos,
            }
        )
    print(f"bundle={major}.{minor} id={bundle_id} entries={count} header={header_pos}")
    return major, result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("bundle", type=Path)
    parser.add_argument("--replace")
    parser.add_argument("--with-file", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--extract")
    args = parser.parse_args()

    data = bytearray(args.bundle.read_bytes())
    major, manifest = entries(data)
    if args.extract:
        entry = next((item for item in manifest if item["name"] == args.extract), None)
        if entry is None or not args.output:
            parser.error("--extract requires a valid entry name and --output")
        offset = int(entry["offset"])
        compressed_size = int(entry["compressed_size"])
        if compressed_size:
            content = zlib.decompress(
                bytes(data[offset : offset + compressed_size]), -zlib.MAX_WBITS
            )
        else:
            content = bytes(data[offset : offset + int(entry["size"])])
        args.output.write_bytes(content)
        print(f"extracted={entry['name']} size={len(content)} output={args.output}")
        return

    if not args.replace:
        for entry in manifest:
            if "Interprocess" in str(entry["name"]) or "Resonite" in str(entry["name"]):
                print(entry)
        return

    if not args.with_file or not args.output:
        parser.error("--replace requires --with-file and --output")
    entry = next((item for item in manifest if item["name"] == args.replace), None)
    if entry is None:
        raise ValueError(f"bundle entry not found: {args.replace}")

    replacement = args.with_file.read_bytes()
    old_payload_size = int(entry["compressed_size"]) or int(entry["size"])
    if int(entry["compressed_size"]):
        compressor = zlib.compressobj(level=9, wbits=-zlib.MAX_WBITS)
        payload = compressor.compress(replacement) + compressor.flush()
    else:
        payload = replacement
    print(
        f"replace={entry['name']} old_slot={old_payload_size} "
        f"new_payload={len(payload)} new_size={len(replacement)}"
    )
    if len(payload) > old_payload_size:
        raise ValueError("replacement does not fit in the existing bundle slot")

    offset = int(entry["offset"])
    data[offset : offset + len(payload)] = payload
    data[offset + len(payload) : offset + old_payload_size] = b"\0" * (
        old_payload_size - len(payload)
    )
    metadata_pos = int(entry["metadata_pos"])
    struct.pack_into("<q", data, metadata_pos + 8, len(replacement))
    if major >= 6:
        struct.pack_into(
            "<q", data, metadata_pos + 16, len(payload) if int(entry["compressed_size"]) else 0
        )
    args.output.write_bytes(data)
    args.output.chmod(args.bundle.stat().st_mode)


if __name__ == "__main__":
    main()
