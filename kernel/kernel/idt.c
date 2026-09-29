#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <stdint.h>

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

extern void idt_flush(struct idt_ptr*);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
	idt[num].offset_low = base & 0xFFFF;
	idt[num].selector = selector;
	idt[num].zero = 0; // reserved by 32bit gate format
	idt[num].flags = flags;
	idt[num].offset_high = (base >> 16) & 0xFFFF;
}

void idt_initialize(void) {
	idtp.limit = sizeof(idt) - 1;
	idtp.base = (uint32_t)&idt;

	for (int i = 0; i < IDT_ENTRIES; i++) {
		idt[i].offset_low = 0;
		idt[i].selector = 0;
		idt[i].zero = 0;
		idt[i].flags = 0;
		idt[i].offset_high = 0;
	}

	// stub table: https://wiki.osdev.org/Interrupt_Descriptor_Table#IDT_items
	for (int i = 0; i < 32; i++) {
		idt_set_gate(i, (uint32_t)isr_stub_table[i], GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);
	}

	idt_flush(&idtp);
}
