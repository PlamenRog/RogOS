#include <kernel/gdt.h>
#include <kernel/tss.h>

#include <string.h>

static struct tss_entry tss;

void tss_initialize(void) {
	memset(&tss, 0, sizeof(tss));

	uint32_t base = (uint32_t)&tss;

	uint32_t limit = sizeof(tss) - 1;

	// 0x80 = present; 0x09 = available 32bit tss
	gdt_set_gate(5, base, limit, 0x89, 0);

	tss.ss0 = GDT_KERNEL_DATA_SELECTOR;
	tss.iomap_base = sizeof(struct tss_entry);

	// gdt entry 5
	uint16_t selector = GDT_TSS_SELECTOR;

	asm volatile("ltr %0" : : "r"(selector) : "memory");
}

void tss_set_kernel_stack(uint32_t stack) {
	tss.esp0 = stack;
}
