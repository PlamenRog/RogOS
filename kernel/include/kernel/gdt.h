#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H

#include <stdint.h>

// access byte
#define GDT_ACCESS_PRESENT 0x80

#define GDT_ACCESS_RING0 0x00
#define GDT_ACCESS_RING1 0x20
#define GDT_ACCESS_RING2 0x40
#define GDT_ACCESS_RING3 0x60

#define GDT_ACCESS_CODEDATA 0x10

#define GDT_ACCESS_EXECUTABLE 0x08
#define GDT_ACCESS_DIRECTION 0x04
#define GDT_ACCESS_READWRITE 0x02
#define GDT_ACCESS_ACCESSED 0x01

#define GDT_KERNEL_CODE_SELECTOR 0x08
#define GDT_KERNEL_DATA_SELECTOR 0x10

#define GDT_USER_CODE_SELECTOR 0x1B
#define GDT_USER_DATA_SELECTOR 0x23

#define GDT_TSS_SELECTOR 0x28

// descriptors
#define GDT_ACCESS_KERNEL_CODE                                                             \
	(GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODEDATA | GDT_ACCESS_EXECUTABLE | \
	 GDT_ACCESS_READWRITE)

#define GDT_ACCESS_KERNEL_DATA \
	(GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODEDATA | GDT_ACCESS_READWRITE)

#define GDT_ACCESS_USER_CODE                                                               \
	(GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODEDATA | GDT_ACCESS_EXECUTABLE | \
	 GDT_ACCESS_READWRITE)

#define GDT_ACCESS_USER_DATA \
	(GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODEDATA | GDT_ACCESS_READWRITE)

// granularity flags
#define GDT_FLAG_4K 0x80
#define GDT_FLAG_32BIT 0x40

#define GDT_FLAGS (GDT_FLAG_4K | GDT_FLAG_32BIT)

struct gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

void gdt_initialize(void);
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);

#endif
