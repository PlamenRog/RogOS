#include <kernel/elf.h>
#include <kernel/elf_loader.h>
#include <kernel/paging.h>
#include <kernel/process.h>
#include <kernel/vfs.h>

#include <stdint.h>
#include <string.h>

int elf_load(struct process* process, const char* path) {
	struct vfs_node* file = vfs_find(path);

	if (!file) {
		return -1;
	}

	struct elf_header header;

	if (vfs_read(file, 0, sizeof(header), (uint8_t*)&header) != sizeof(header)) {
		return -1;
	}
	if (file->size < sizeof(struct elf_header)) {
		return -1;
	}
	if (header.magic != ELF_MAGIC) {
		return -1;
	}
	if (header.ident[0] != ELF_CLASS_32) {
		return -1;
	}
	if (header.ident[1] != ELF_DATA_LSB) {
		return -1;
	}
	if (header.ident[2] != ELF_VERSION_CURRENT) {
		return -1;
	}
	if (header.type != ELF_ET_EXEC) {
		return -1;
	}
	if (header.machine != ELF_EM_386) {
		return -1;
	}
	if (header.version != ELF_VERSION_CURRENT) {
		return -1;
	}
	if (header.ehsize != sizeof(struct elf_header)) {
		return -1;
	}
	if (header.phentsize != sizeof(struct elf_program_header)) {
		return -1;
	}
	if (header.phnum == 0) {
		return -1;
	}
	if (header.phoff > file->size) {
		return -1;
	}

	uint32_t available = file->size - header.phoff;

	if (header.phnum > available / header.phentsize) {
		return -1;
	}

	int loaded_segment = 0;
	int entry_valid = 0;

	for (uint32_t i = 0; i < header.phnum; i++) {
		struct elf_program_header ph;

		uint32_t offset = header.phoff + i * header.phentsize;

		if (vfs_read(file, offset, sizeof(ph), (uint8_t*)&ph) != sizeof(ph)) {
			return -1;
		}
		if (ph.type != ELF_PT_LOAD) {
			continue;
		}
		if (ph.file_size > ph.memory_size) {
			return -1;
		}
		if (ph.memory_size == 0) {
			continue;
		}
		if (ph.virtual_address & PAGE_OFFSET_MASK) {
			return -1;
		}
		if (ph.offset > file->size) {
			return -1;
		}
		if (ph.file_size > file->size - ph.offset) {
			return -1;
		}
		if (ph.virtual_address < USER_PROGRAM_MIN) {
			return -1;
		}
		if (ph.virtual_address >= USER_STACK_BOTTOM) {
			return -1;
		}
		if (ph.memory_size > USER_STACK_BOTTOM - ph.virtual_address) {
			return -1;
		}

		loaded_segment = 1;

		if ((ph.flags & ELF_PF_X) && header.entry >= ph.virtual_address && header.entry - ph.virtual_address < ph.memory_size) {
			entry_valid = 1;
		}

		uint32_t final_flags = 0;

		if (ph.flags & ELF_PF_R) {
			final_flags |= REGION_READ;
		}
		if (ph.flags & ELF_PF_W) {
			final_flags |= REGION_WRITE;
		}
		if (ph.flags & ELF_PF_X) {
			final_flags |= REGION_EXEC;
		}
		if (process_map_region(process, ph.virtual_address, ph.memory_size, final_flags | REGION_WRITE) != 0) {
			return -1;
		}

		paging_switch_directory(process->page_directory);

		if (ph.file_size > 0) {
			if (vfs_read(file, ph.offset, ph.file_size, (uint8_t*)ph.virtual_address) != ph.file_size) {
				paging_switch_directory(paging_kernel_directory());

				return -1;
			}
		}

		if (ph.memory_size > ph.file_size) {
			memset((void*)(ph.virtual_address + ph.file_size), 0, ph.memory_size - ph.file_size);
		}

		paging_switch_directory(paging_kernel_directory());

		if (process_set_region_flags(process, ph.virtual_address, ph.memory_size, final_flags) != 0) {
			return -1;
		}
	}

	if (!loaded_segment) {
		return -1;
	}
	if (!entry_valid) {
		return -1;
	}
	if (process_map_region(process, USER_STACK_TOP - USER_STACK_SIZE, USER_STACK_SIZE, REGION_READ | REGION_WRITE) != 0) {
		return -1;
	}

	process->user_stack = USER_STACK_TOP;
	process->entry = header.entry;

	return 0;
}
