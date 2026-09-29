#!/bin/sh
set -e

. ./headers.sh

(
    cd libc

    DESTDIR="$SYSROOT" \
        $MAKE install
)

$MAKE \
    -C userspace \
    HOST="$HOST" \
    SYSROOT="$SYSROOT"

mkdir -p build

python3 tools/mkramfs.py \
    "$SYSROOT/usr/bin" \
    build/initramfs.bin

(
    cd build

    ${HOST}-objcopy \
        -I binary \
        -O elf32-i386 \
        -B i386 \
        initramfs.bin \
        initramfs.o
)

(
    cd kernel

    DESTDIR="$SYSROOT" \
        $MAKE install
)
