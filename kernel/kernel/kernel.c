#include <stdio.h>

#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/tss.h>
#include <kernel/tty.h>

#include <kernel/heap.h>
#include <kernel/irq.h>
#include <kernel/multiboot.h>
#include <kernel/paging.h>
#include <kernel/pic.h>
#include <kernel/pmm.h>
#include <kernel/process.h>
#include <kernel/ramfs.h>
#include <kernel/shell.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>

void kernel_main(uint32_t magic, uint32_t addr) {
	terminal_initialize();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		terminal_writestring("Invalid Multiboot boot magic\n");

		for (;;) {
			__asm__ volatile("cli; hlt");
		}
	}

	gdt_initialize();

	tss_initialize();

	idt_initialize();

	pmm_initialize(addr);

	paging_initialize();

	heap_initialize();

	if (ramfs_initialize() != 0) {
		terminal_writestring("Failed to initialize RAMFS\n");

		for (;;) {
			__asm__ volatile("cli; hlt");
		}
	}

	vfs_mount(ramfs_root());

	syscall_initialize();

	pic_remap();
	irq_initialize();

	shell_initialize();

	__asm__ volatile("sti");

	shell_run();

	for (;;) {
		__asm__ volatile("hlt");
	}
}
