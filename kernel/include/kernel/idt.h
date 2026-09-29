#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include <stdint.h>

struct idt_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t flags;
	uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

#define IDT_FLAG_PRESENT 0x80

#define IDT_FLAG_RING0 0x00
#define IDT_FLAG_RING1 0x20
#define IDT_FLAG_RING2 0x40
#define IDT_FLAG_RING3 0x60

#define IDT_FLAG_INTERRUPT_GATE 0x0E
#define IDT_FLAG_TRAP_GATE 0x0F

#define IDT_KERNEL_GATE (IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INTERRUPT_GATE)
#define IDT_SYSCALL_GATE (IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_INTERRUPT_GATE)

void idt_initialize(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

#endif
