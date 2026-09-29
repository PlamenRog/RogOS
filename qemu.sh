#!/bin/sh
set -e

. ./config.sh

./iso.sh

qemu-system-$(./target-triplet-to-arch.sh "$HOST") \
    -cdrom rogos.iso \
    -no-reboot \
    -d int,cpu_reset \
    -display gtk,zoom-to-fit=on
