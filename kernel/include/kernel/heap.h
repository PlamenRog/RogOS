#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <stddef.h>
#include <stdint.h>

void heap_initialize(void);

void* kmalloc(size_t size);

void kfree(void* ptr);

#endif
