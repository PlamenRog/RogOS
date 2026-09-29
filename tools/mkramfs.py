#!/usr/bin/env python3

import os
import struct
import sys

if len(sys.argv) != 3:
    print(f"usage: {sys.argv[0]} <input-directory> <output>")
    sys.exit(1)

input_dir = sys.argv[1]
output_path = sys.argv[2]

files = []

for name in sorted(os.listdir(input_dir)):
    path = os.path.join(input_dir, name)

    if not os.path.isfile(path):
        continue

    with open(path, "rb") as f:
        data = f.read()

    files.append((name.encode("utf-8"), data))

with open(output_path, "wb") as out:
    out.write(b"RFS1")
    out.write(struct.pack("<I", len(files)))

    for name, data in files:
        out.write(struct.pack("<I", len(name)))
        out.write(struct.pack("<I", len(data)))
        out.write(name)
        out.write(data)
