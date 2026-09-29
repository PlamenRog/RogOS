#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <string.h>

#define KERNEL_IDENTITY_TABLES 16u
#define KERNEL_SHARED_TABLES 17u

#define KERNEL_IDENTITY_LIMIT (KERNEL_IDENTITY_TABLES * PAGE_TABLE_ENTRIES * PAGE_SIZE)

static struct page_directory kernel_directory __attribute__((aligned(PAGE_SIZE)));

static uint32_t identity_tables[KERNEL_IDENTITY_TABLES][PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

// plans to set for: https://wiki.osdev.org/X86_Paging#32-bit_Paging_(Protected_Mode)
void paging_initialize(void) {
	memset(&kernel_directory, 0, sizeof(struct page_directory));

	for (uint32_t table = 0; table < KERNEL_IDENTITY_TABLES; table++) {
		for (uint32_t entry = 0; entry < PAGE_TABLE_ENTRIES; entry++) {
			identity_tables[table][entry] = ((table * PAGE_TABLE_ENTRIES + entry) * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
		}

		kernel_directory.entries[table] = ((uint32_t)identity_tables[table]) | PAGE_PRESENT | PAGE_WRITABLE;
	}

	paging_switch_directory(&kernel_directory);

	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));

	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

struct page_directory* paging_kernel_directory(void) {
	return &kernel_directory;
}

struct page_directory* paging_create_directory(void) {
	struct page_directory* directory = (struct page_directory*)pmm_alloc_frame_below(KERNEL_IDENTITY_LIMIT);

	if (!directory) {
		return 0;
	}

	memset(directory, 0, PAGE_SIZE);

	for (uint32_t i = 0; i < KERNEL_SHARED_TABLES; i++) {
		directory->entries[i] = kernel_directory.entries[i];
	}

	return directory;
}

void paging_switch_directory(struct page_directory* directory) {
	uint32_t address = (uint32_t)directory;

	asm volatile("mov %0, %%cr3" : : "r"(address) : "memory");
}

int paging_set_page_flags(struct page_directory* directory, uint32_t virtual_address,
						  uint32_t flags) {
	if (!directory) {
		return -1;
	}

	if (virtual_address & PAGE_OFFSET_MASK) {
		return -1;
	}

	uint32_t directory_index = virtual_address >> PAGE_DIRECTORY_SHIFT;

	uint32_t table_index = (virtual_address >> PAGE_SHIFT) & PAGE_INDEX_MASK;

	uint32_t directory_entry = directory->entries[directory_index];

	if (!(directory_entry & PAGE_PRESENT)) {
		return -1;
	}

	uint32_t* table = (uint32_t*)(directory_entry & PAGE_FRAME_MASK);

	uint32_t page = table[table_index];

	if (!(page & PAGE_PRESENT)) {
		return -1;
	}

	uint32_t frame = page & PAGE_FRAME_MASK;

	uint32_t page_flags = PAGE_PRESENT;

	if (flags & PAGE_WRITABLE) {
		page_flags |= PAGE_WRITABLE;
	}

	if (flags & PAGE_USER) {
		page_flags |= PAGE_USER;
	}

	table[table_index] = frame | page_flags;

	asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");

	return 0;
}

void paging_destroy_directory(struct page_directory* directory) {
	if (!directory) {
		return;
	}

	// pdes 0-16 are shared page tables
	for (uint32_t i = KERNEL_SHARED_TABLES; i < PAGE_DIRECTORY_ENTRIES; i++) {
		uint32_t entry = directory->entries[i];

		if (!(entry & PAGE_PRESENT)) {
			continue;
		}

		uint32_t* table = (uint32_t*)(entry & PAGE_FRAME_MASK);

		// free user frames
		for (uint32_t j = 0; j < PAGE_TABLE_ENTRIES; j++) {
			uint32_t page = table[j];

			if (!(page & PAGE_PRESENT)) {
				continue;
			}

			if (!(page & PAGE_USER)) {
				continue;
			}

			uint32_t frame = page & PAGE_FRAME_MASK;

			pmm_free_frame(frame);
		}

		pmm_free_frame(entry & PAGE_FRAME_MASK);

		directory->entries[i] = 0;
	}

	pmm_free_frame((uint32_t)directory);
}

int paging_map_page(struct page_directory* directory, uint32_t virtual, uint32_t physical, uint32_t flags) {
	if (!directory) {
		return -1;
	}

	if (virtual & PAGE_OFFSET_MASK) {
		return -1;
	}

	if (physical & PAGE_OFFSET_MASK) {
		return -1;
	}

	uint32_t directory_index = virtual >> PAGE_DIRECTORY_SHIFT;

	uint32_t table_index = (virtual >> PAGE_SHIFT) & PAGE_INDEX_MASK;

	uint32_t* table;

	uint32_t directory_entry = directory->entries[directory_index];

	if (!(directory_entry & PAGE_PRESENT)) {
		uint32_t table_frame = pmm_alloc_frame_below(KERNEL_IDENTITY_LIMIT);

		if (!table_frame) {
			return -1;
		}

		table = (uint32_t*)table_frame;

		memset(table, 0, PAGE_SIZE);

		uint32_t directory_flags = PAGE_PRESENT | PAGE_WRITABLE;

		if (flags & PAGE_USER) {
			directory_flags |= PAGE_USER;
		}

		directory->entries[directory_index] = table_frame | directory_flags;
	}
	else {
		table = (uint32_t*)(directory_entry & PAGE_FRAME_MASK);

		if (flags & PAGE_USER) {
			directory->entries[directory_index] |= PAGE_USER;
		}
	}

	if (table[table_index] & PAGE_PRESENT) {
		return -1;
	}

	uint32_t page_flags = PAGE_PRESENT;

	if (flags & PAGE_WRITABLE) {
		page_flags |= PAGE_WRITABLE;
	}
	if (flags & PAGE_USER) {
		page_flags |= PAGE_USER;
	}

	table[table_index] = physical | page_flags;

	asm volatile("invlpg (%0)" : : "r"(virtual) : "memory");

	return 0;
}

int paging_unmap_page(struct page_directory* directory, uint32_t virtual_address, uint32_t* physical_out) {
	if (!directory) {
		return -1;
	}

	if (virtual_address & PAGE_OFFSET_MASK) {
		return -1;
	}

	uint32_t directory_index = virtual_address >> PAGE_DIRECTORY_SHIFT;

	uint32_t table_index = (virtual_address >> PAGE_SHIFT) & PAGE_INDEX_MASK;

	uint32_t directory_entry = directory->entries[directory_index];

	if (!(directory_entry & PAGE_PRESENT)) {
		return -1;
	}

	uint32_t* table = (uint32_t*)(directory_entry & PAGE_FRAME_MASK);

	uint32_t page = table[table_index];

	if (!(page & PAGE_PRESENT)) {
		return -1;
	}

	if (physical_out) {
		*physical_out = page & PAGE_FRAME_MASK;
	}

	table[table_index] = 0;

	asm volatile("invlpg (%0)" : : "r"(virtual_address) : "memory");

	return 0;
}

int paging_user_accessible(struct page_directory* directory, uint32_t virtual_address, int write) {
	if (!directory) {
		return 0;
	}

	uint32_t directory_index = virtual_address >> PAGE_DIRECTORY_SHIFT;

	uint32_t table_index = (virtual_address >> PAGE_SHIFT) & PAGE_INDEX_MASK;

	uint32_t pde = directory->entries[directory_index];

	if (!(pde & PAGE_PRESENT)) {
		return 0;
	}
	if (!(pde & PAGE_USER)) {
		return 0;
	}

	uint32_t* table = (uint32_t*)(pde & PAGE_FRAME_MASK);
	uint32_t pte = table[table_index];

	if (!(pte & PAGE_PRESENT)) {
		return 0;
	}
	if (!(pte & PAGE_USER)) {
		return 0;
	}
	if (write && !(pte & PAGE_WRITABLE)) {
		return 0;
	}

	return 1;
}
