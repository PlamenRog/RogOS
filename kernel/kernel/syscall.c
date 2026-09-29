#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/keyboard.h>
#include <kernel/paging.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/tty.h>
#include <kernel/vfs.h>

extern void syscall_stub(void);

void syscall_initialize(void) {
	idt_set_gate(0x80, (uint32_t)syscall_stub, GDT_KERNEL_CODE_SELECTOR, IDT_SYSCALL_GATE);
}

static int user_range_valid(struct process* process, const void* pointer, uint32_t length, int write) {
	if (!process || !process->page_directory) {
		return 0;
	}

	if (length == 0) {
		return 1;
	}

	if (!pointer) {
		return 0;
	}

	uint32_t start = (uint32_t)pointer;

	if (length - 1 > UINT32_MAX - start) {
		return 0;
	}

	uint32_t end = start + length - 1;
	uint32_t page = start & ~PAGE_OFFSET_MASK;

	for (;;) {
		if (!paging_user_accessible(process->page_directory, page, write)) {
			return 0;
		}

		if (end - page < PAGE_SIZE) {
			break;
		}
		if (page > UINT32_MAX - PAGE_SIZE) {
			return 0;
		}

		page += PAGE_SIZE;
	}

	return 1;
}

static int copy_user_string(struct process* process, const char* user_string, char* kernel_buffer, uint32_t capacity) {
	if (!process || !user_string || !kernel_buffer || capacity == 0) {
		return -1;
	}

	for (uint32_t i = 0; i < capacity; i++) {
		const char* address = user_string + i;

		if (!paging_user_accessible(process->page_directory, (uint32_t)address, 0)) {
			return -1;
		}

		char c = *address;

		kernel_buffer[i] = c;

		if (c == '\0') {
			return 0;
		}
	}

	kernel_buffer[capacity - 1] = '\0';

	return -1;
}

static uint32_t fd_flags_from_open_flags(int flags) {
	switch (flags & 0x3) {
	case O_RDONLY:
		return FD_FLAG_READ;

	case O_WRONLY:
		return FD_FLAG_WRITE;

	case O_RDWR:
		return FD_FLAG_READ | FD_FLAG_WRITE;

	default:
		return 0;
	}
}

static int syscall_write(int fd, const char* buffer, uint32_t length) {
	struct process* process = process_get_current();

	if (!process) {
		return -1;
	}
	if (length == 0) {
		return 0;
	}
	if (!user_range_valid(process, buffer, length, 0)) {
		return -1;
	}

	if (fd == 1 || fd == 2) {
		terminal_write(buffer, length);

		return (int)length;
	}

	struct process_fd* file = process_fd_get(process, fd);

	if (!file) {
		return -1;
	}

	if (!(file->flags & FD_FLAG_WRITE)) {
		return -1;
	}

	uint32_t written = vfs_write(file->node, file->offset, length, (uint8_t*)buffer);

	file->offset += written;

	return (int)written;
}

static int syscall_read(int fd, char* buffer, uint32_t length) {
	struct process* process = process_get_current();

	if (!process) {
		return -1;
	}
	if (length == 0) {
		return 0;
	}

	if (!user_range_valid(process, buffer, length, 1)) {
		return -1;
	}

	if (fd == 0) {
		for (uint32_t i = 0; i < length; i++) {
			buffer[i] = keyboard_getchar(); // keyboard_getchar waits with hlt and needs interrupt to wake us up
		}

		// return to syscall with disabled interrupts
		return (int)length;
	}

	struct process_fd* file = process_fd_get(process, fd);

	if (!file) {
		return -1;
	}

	if (!(file->flags & FD_FLAG_READ)) {
		return -1;
	}

	uint32_t count = vfs_read(file->node, file->offset, length, (uint8_t*)buffer);

	file->offset += count;

	return (int)count;
}

static int syscall_open(const char* path, int flags) {
	if (!path) {
		return -1;
	}

	struct process* process = process_get_current();

	if (!process) {
		return -1;
	}

	uint32_t fd_flags = fd_flags_from_open_flags(flags);

	if (fd_flags == 0) {
		return -1;
	}
	if (flags & O_TRUNC) {
		int access_mode = flags & 0x3;

		if (access_mode != O_WRONLY && access_mode != O_RDWR) {
			return -1;
		}
	}

	int fd_available = 0;

	for (int fd = 3; fd < PROCESS_MAX_FDS; fd++) {
		if (!process->fds[fd].used) {
			fd_available = 1;
			break;
		}
	}

	if (!fd_available) {
		return -1;
	}

	struct vfs_node* node = vfs_find(path);

	if (!node) {
		if (!(flags & O_CREAT)) {
			return -1;
		}

		node = vfs_create(path);

		if (!node) {
			return -1;
		}
	}

	if (flags & O_TRUNC) {
		if (vfs_truncate(node) != 0) {
			return -1;
		}
	}

	return process_fd_allocate(process, node, fd_flags);
}

static int syscall_close(int fd) {
	struct process* process = process_get_current();

	if (!process) {
		return -1;
	}

	return process_fd_close(process, fd);
}

void syscall_handler(struct registers* regs) {
	if (!regs) {
		return;
	}

	switch (regs->eax) {
	case SYS_EXIT:
		process_exit((int)regs->ebx);

		__builtin_unreachable();

	case SYS_WRITE:
		regs->eax = syscall_write((int)regs->ebx, (const char*)regs->ecx, regs->edx);

		return;

	case SYS_OPEN: {
		struct process* process = process_get_current();

		if (!process) {
			regs->eax = (uint32_t)-1;
			return;
		}

		char path[VFS_MAX_NAME];

		if (copy_user_string(process, (const char*)regs->ebx, path, sizeof(path)) != 0) {
			regs->eax = (uint32_t)-1;
			return;
		}

		regs->eax = syscall_open(path, (int)regs->ecx);

		return;
	}

	case SYS_CLOSE:
		regs->eax = syscall_close((int)regs->ebx);

		return;

	case SYS_READ:
		regs->eax = syscall_read((int)regs->ebx, (char*)regs->ecx, regs->edx);

		return;

	default:
		printf("Unknown syscall: %u\n", regs->eax);

		regs->eax = (uint32_t)-1;

		return;
	}
}
