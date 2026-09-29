#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE 0x1000u
#define PAGE_SHIFT 12u
#define PAGE_OFFSET_MASK 0x0FFFu
#define PAGE_FRAME_MASK 0xFFFFF000u

#define PAGE_TABLE_ENTRIES 1024u
#define PAGE_DIRECTORY_ENTRIES 1024u
#define PAGE_DIRECTORY_SHIFT 22u
#define PAGE_INDEX_MASK 0x03FFu

#define PAGE_PRESENT 0x001u
#define PAGE_WRITABLE 0x002u
#define PAGE_USER 0x004u

struct page_directory {
	uint32_t entries[PAGE_DIRECTORY_ENTRIES];
} __attribute__((aligned(PAGE_SIZE)));

void paging_initialize(void);

struct page_directory* paging_kernel_directory(void);
struct page_directory* paging_create_directory(void);

void paging_switch_directory(struct page_directory* directory);

void paging_destroy_directory(struct page_directory* directory);

int paging_map_page(struct page_directory* directory, uint32_t virtual, uint32_t physical, uint32_t flags);
int paging_unmap_page(struct page_directory* directory, uint32_t virtual_address, uint32_t* physical_out);

int paging_set_page_flags(struct page_directory* directory, uint32_t virtual_address, uint32_t flags);

int paging_user_accessible(struct page_directory* directory, uint32_t virtual_address, int write);

#endif
