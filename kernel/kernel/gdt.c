#include <stdint.h>

#include <kernel/gdt.h>

#define GDT_ENTRIES 6

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gp;

extern void gdt_flush(struct gdt_ptr*);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
	gdt[num].base_low = base & 0xFFFF;
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	gdt[num].limit_low = limit & 0xFFFF;
	gdt[num].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
	gdt[num].access = access;
}

void gdt_initialize(void) {
	gp.limit = sizeof(gdt) - 1;

	gp.base = (uint32_t)&gdt;

	// all slots are as described in: https://wiki.osdev.org/GDT_Tutorial#Flat_/_Long_Mode_Setup
	// null descriptor
	gdt_set_gate(0, 0, 0, 0, 0);

	// kernel code
	gdt_set_gate(1, 0, 0xFFFFFFFF, GDT_ACCESS_KERNEL_CODE, GDT_FLAGS);

	// kernel data
	gdt_set_gate(2, 0, 0xFFFFFFFF, GDT_ACCESS_KERNEL_DATA, GDT_FLAGS);

	// user code
	gdt_set_gate(3, 0, 0xFFFFFFFF, GDT_ACCESS_USER_CODE, GDT_FLAGS);

	// user data
	gdt_set_gate(4, 0, 0xFFFFFFFF, GDT_ACCESS_USER_DATA, GDT_FLAGS);

	gdt_flush(&gp);
}
