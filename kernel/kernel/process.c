#include <kernel/context.h>
#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <kernel/process.h>
#include <kernel/tss.h>

#include <stdio.h>
#include <string.h>

#define PROCESS_KERNEL_STACK_SIZE 8192

static struct process* current_process = 0;
static uint32_t next_pid = 1;

extern void enter_user_mode(uint32_t entry, uint32_t user_stack) __attribute__((noreturn));

static void process_trampoline(void) {
	struct process* process = process_get_current();

	if (!process) {
		for (;;) {
			asm volatile("cli; hlt");
		}
	}

	enter_user_mode(process->entry, process->user_stack);

	__builtin_unreachable();
}

int process_set_region_flags(struct process* process, uint32_t address, uint32_t size,
							 uint32_t region_flags) {
	if (!process || !process->page_directory || size == 0) {
		return -1;
	}

	if (address & PAGE_OFFSET_MASK) {
		return -1;
	}

	uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	uint32_t page_flags = PAGE_USER;

	if (region_flags & REGION_WRITE) {
		page_flags |= PAGE_WRITABLE;
	}

	for (uint32_t i = 0; i < pages; i++) {
		if (paging_set_page_flags(process->page_directory, address + i * PAGE_SIZE, page_flags) != 0) {
			return -1;
		}
	}

	return 0;
}

struct process* process_create(const char* name) {
	struct process* process = kmalloc(sizeof(struct process));

	if (!process) {
		return 0;
	}

	memset(process, 0, sizeof(struct process));

	process->pid = next_pid++;

	strncpy(process->name, name, PROCESS_NAME_MAX - 1);

	process->name[PROCESS_NAME_MAX - 1] = '\0';

	process->fds[0].used = 1;
	process->fds[0].flags = FD_FLAG_READ;

	process->fds[1].used = 1;
	process->fds[1].flags = FD_FLAG_WRITE;

	process->fds[2].used = 1;
	process->fds[2].flags = FD_FLAG_WRITE;

	process->page_directory = paging_create_directory();

	if (!process->page_directory) {
		process_destroy(process);
		return 0;
	}

	uint32_t stack = (uint32_t)kmalloc(PROCESS_KERNEL_STACK_SIZE);

	if (!stack) {
		process_destroy(process);
		return 0;
	}

	process->kernel_stack = stack + PROCESS_KERNEL_STACK_SIZE;

	return process;
}

struct process* process_get_current(void) {
	return current_process;
}

int process_allocate_page(struct process* process, uint32_t virtual_address, uint32_t region_flags) {
	if (!process || !process->page_directory) {
		return -1;
	}

	uint32_t frame = pmm_alloc_frame();

	if (!frame) {
		return -1;
	}

	uint32_t page_flags = PAGE_USER;

	if (region_flags & REGION_WRITE) {
		page_flags |= PAGE_WRITABLE;
	}

	if (paging_map_page(process->page_directory, virtual_address, frame, page_flags) != 0) {
		pmm_free_frame(frame);
		return -1;
	}

	paging_switch_directory(process->page_directory);

	memset((void*)virtual_address, 0, PAGE_SIZE);

	paging_switch_directory(paging_kernel_directory());

	return 0;
}

int process_map_region(struct process* process, uint32_t address, uint32_t size, uint32_t flags) {
	if (!process || size == 0) {
		return -1;
	}

	uint32_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint32_t i = 0; i < pages; i++) {
		if (process_allocate_page(process, address + i * PAGE_SIZE, flags) != 0) {
			return -1;
		}
	}

	return 0;
}

struct process_fd* process_fd_get(struct process* process, int fd) {
	if (!process) {
		return 0;
	}

	if (fd < 0 || fd >= PROCESS_MAX_FDS) {
		return 0;
	}

	if (!process->fds[fd].used) {
		return 0;
	}

	return &process->fds[fd];
}

int process_fd_allocate(struct process* process, struct vfs_node* node, uint32_t flags) {
	if (!process || !node) {
		return -1;
	}

	if (!(flags & (FD_FLAG_READ | FD_FLAG_WRITE))) {
		return -1;
	}

	for (int fd = 3; fd < PROCESS_MAX_FDS; fd++) {
		if (process->fds[fd].used) {
			continue;
		}

		struct process_fd* entry = &process->fds[fd];

		entry->used = 1;
		entry->node = node;
		entry->offset = 0;
		entry->flags = flags;

		return fd;
	}

	return -1;
}

int process_fd_close(struct process* process, int fd) {
	struct process_fd* entry = process_fd_get(process, fd);

	if (!entry) {
		return -1;
	}

	if (fd < 3) {
		return 0;
	}

	memset(entry, 0, sizeof(struct process_fd));

	return 0;
}

void process_start(struct process* process) {
	if (!process) {
		return;
	}

	current_process = process;

	paging_switch_directory(process->page_directory);

	tss_set_kernel_stack(process->kernel_stack);

	uint32_t stack = process->kernel_stack & ~0x0F;

	process->context.edi = 0;
	process->context.esi = 0;
	process->context.ebp = 0;
	process->context.esp = stack;
	process->context.ebx = 0;

	process->context.eip = (uint32_t)process_trampoline;

	process->context.eflags = 0x202;

	context_switch(&process->parent_context, &process->context);

	paging_switch_directory(paging_kernel_directory());

	current_process = 0;
	tss_set_kernel_stack(0);
}

void process_exit(int status) {
	struct process* process = process_get_current();

	if (!process) {
		printf("process_exit: no current process\n");

		for (;;) {
			asm volatile("cli; hlt");
		}
	}

	printf("Process %u exited with status %d\n", process->pid, status);

	paging_switch_directory(paging_kernel_directory());

	context_switch(&process->context, &process->parent_context);

	for (;;) {
		asm volatile("cli; hlt");
	}
}

void process_destroy(struct process* process) {
	if (!process) {
		return;
	}
	if (process == current_process) {
		return;
	}

	if (process->page_directory) {
		paging_destroy_directory(process->page_directory);

		process->page_directory = 0;
	}

	if (process->kernel_stack) {
		uint32_t stack_base = process->kernel_stack - PROCESS_KERNEL_STACK_SIZE;

		kfree((void*)stack_base);

		process->kernel_stack = 0;
	}

	kfree(process);
}
