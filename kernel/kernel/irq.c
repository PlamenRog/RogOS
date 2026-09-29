#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/irq.h>
#include <kernel/isr.h>
#include <kernel/keyboard.h>
#include <kernel/pic.h>

void irq_initialize(void) {
	idt_set_gate(32, (uint32_t)irq0, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(33, (uint32_t)irq1, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(34, (uint32_t)irq2, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(35, (uint32_t)irq3, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(36, (uint32_t)irq4, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(37, (uint32_t)irq5, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(38, (uint32_t)irq6, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(39, (uint32_t)irq7, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(40, (uint32_t)irq8, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(41, (uint32_t)irq9, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(42, (uint32_t)irq10, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(43, (uint32_t)irq11, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(44, (uint32_t)irq12, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(45, (uint32_t)irq13, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(46, (uint32_t)irq14, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);

	idt_set_gate(47, (uint32_t)irq15, GDT_KERNEL_CODE_SELECTOR, IDT_KERNEL_GATE);
}

void irq_handler(struct registers* regs) {

	// printf("IRQ %u\n", regs->int_no - 32);

	switch (regs->int_no) {
	case 33:
		keyboard_handler();
		break;

	default:
		break;
	}

	pic_send_eoi(regs->int_no - 32);
}
