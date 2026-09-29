#ifndef KERNEL_INITRAMFS_H
#define KERNEL_INITRAMFS_H

#include <stdint.h>

extern uint8_t _binary_initramfs_bin_start[];
extern uint8_t _binary_initramfs_bin_end[];

#define INITRAMFS_START _binary_initramfs_bin_start
#define INITRAMFS_SIZE ((uint32_t)(_binary_initramfs_bin_end - _binary_initramfs_bin_start))

#endif
