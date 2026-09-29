#include <stdio.h>

#include <kernel/isr.h>
#include <kernel/process.h>
#include <kernel/tty.h>

static const char* exception_messages[32] = {
	"Division By Zero",			   // 0
	"Debug",					   // 1
	"Non Maskable Interrupt",	   // 2
	"Breakpoint",				   // 3
	"Overflow",					   // 4
	"Bound Range Exceeded",		   // 5
	"Invalid Opcode",			   // 6
	"Device Not Available",		   // 7
	"Double Fault",				   // 8
	"Coprocessor Segment Overrun", // 9
	"Invalid TSS",				   // 10
	"Segment Not Present",		   // 11
	"Stack Fault",				   // 12
	"General Protection Fault",	   // 13
	"Page Fault",				   // 14
	"Reserved",					   // 15
	"x87 Floating Point",		   // 16
	"Alignment Check",			   // 17
	"Machine Check",			   // 18
	"SIMD Floating Point",		   // 19
	"Virtualization Exception",	   // 20
	"Control Protection",		   // 21
	"Reserved",					   // 22
	"Reserved",					   // 23
	"Reserved",					   // 24
	"Reserved",					   // 25
	"Reserved",					   // 26
	"Reserved",					   // 27
	"Reserved",					   // 28
	"Reserved",					   // 29
	"Security Exception",		   // 30
	"Reserved"					   // 31
};

static void print_hex(uint32_t value) {
	char hex[] = "0123456789ABCDEF";
	char buffer[9];

	buffer[8] = '\0';

	for (int i = 7; i >= 0; i--) {
		buffer[i] = hex[value & 0xF];
		value >>= 4;
	}

	terminal_writestring(buffer);
}

void isr_handler(struct registers* regs) {
	if (!regs) {
		for (;;) {
			__asm__ volatile("cli; hlt");
		}
	}

	int fatal_exception = regs->int_no == 2 || regs->int_no == 8 || regs->int_no == 18;

	int from_userspace = (regs->cs & 0x3) == 3;

	if (from_userspace && !fatal_exception && process_get_current()) {
		terminal_writestring("Process terminated by CPU exception: ");

		if (regs->int_no < 32) {
			terminal_writestring(exception_messages[regs->int_no]);
		}
		else {
			terminal_writestring("Unknown");
		}

		terminal_putchar('\n');

		process_exit(128 + (int)regs->int_no);

		__builtin_unreachable();
	}

	terminal_writestring("CPU EXCEPTION: ");

	if (regs->int_no < 32) {
		terminal_writestring(exception_messages[regs->int_no]);
	}
	else {
		terminal_writestring("Unknown");
	}

	terminal_putchar('\n');

	terminal_writestring("EIP: 0x");
	print_hex(regs->eip);
	terminal_putchar('\n');

	terminal_writestring("CS: 0x");
	print_hex(regs->cs);
	terminal_putchar('\n');

	terminal_writestring("EFLAGS: 0x");
	print_hex(regs->eflags);
	terminal_putchar('\n');

	for (;;) {
		__asm__ volatile("cli; hlt");
	}
}
