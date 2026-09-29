#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include <stdint.h>

void pmm_initialize(uint32_t multiboot_addr);

uint32_t pmm_alloc_frame(void);

void pmm_free_frame(uint32_t addr);

uint32_t pmm_alloc_frame_below(uint32_t limit);

extern uint32_t kernel_end;

#endif
