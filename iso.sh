#!/bin/sh
set -e

. ./config.sh

./build.sh

mkdir -p isodir/boot/grub

cp "$SYSROOT/boot/rogos.kernel" \
    isodir/boot/rogos.kernel

cat > isodir/boot/grub/grub.cfg << EOF
menuentry "rogos" {
    multiboot /boot/rogos.kernel
}
EOF

grub-mkrescue \
    -o rogos.iso \
    isodir
