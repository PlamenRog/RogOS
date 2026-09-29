#include <kernel/heap.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <kernel/tty.h>
#include <stdio.h>

#define HEAP_START 0x04000000u
#define HEAP_LIMIT 0x04400000u

#define HEAP_ALIGNMENT 16u

struct heap_block {
	size_t size;

	uint8_t free;

	struct heap_block* next;
};

#define HEAP_HEADER_SIZE ((sizeof(struct heap_block) + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1))

static struct heap_block* heap_head;
static uint32_t heap_end;

static void heap_rollback(uint32_t start, uint32_t end) {
	while (end > start) {
		end -= PAGE_SIZE;

		uint32_t frame = 0;

		if (paging_unmap_page(paging_kernel_directory(), end, &frame) == 0) {
			pmm_free_frame(frame);
		}
	}

	heap_end = start;
}

static size_t heap_align_size(size_t size) {
	return (size + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);
}

static int heap_expand(uint32_t size) {
	if (size > UINT32_MAX - HEAP_HEADER_SIZE - (PAGE_SIZE - 1)) {
		return -1;
	}

	uint32_t required = size + HEAP_HEADER_SIZE;
	uint32_t pages = (required + PAGE_SIZE - 1) / PAGE_SIZE;

	if (heap_end > HEAP_LIMIT) {
		return -1;
	}

	uint32_t available_pages = (HEAP_LIMIT - heap_end) / PAGE_SIZE;

	if (pages > available_pages) {
		return -1;
	}

	uint32_t start = heap_end;

	for (uint32_t i = 0; i < pages; i++) {
		uint32_t frame = pmm_alloc_frame();

		if (!frame) {
			heap_rollback(start, heap_end);

			return -1;
		}

		// reverts when frames cant be mapped
		if (paging_map_page(paging_kernel_directory(), heap_end, frame, PAGE_WRITABLE) != 0) {
			pmm_free_frame(frame);
			heap_rollback(start, heap_end);

			return -1;
		}

		heap_end += PAGE_SIZE;
	}

	struct heap_block* block = (struct heap_block*)start;

	block->size = pages * PAGE_SIZE - HEAP_HEADER_SIZE;

	block->free = 1;
	block->next = 0;

	if (!heap_head) {
		heap_head = block;
		return 0;
	}

	struct heap_block* current = heap_head;

	while (current->next) {
		current = current->next;
	}

	current->next = block;

	return 0;
}

void heap_initialize(void) {
	heap_end = HEAP_START;
	heap_head = 0;

	if (heap_expand(PAGE_SIZE) != 0) {
		terminal_writestring("Heap initialization failed\n");

		for (;;) {
			asm volatile("cli; hlt");
		}
	}
}

static void split_block(struct heap_block* block, size_t size) {
	if (block->size < size + HEAP_HEADER_SIZE + HEAP_ALIGNMENT) {
		return;
	}

	struct heap_block* new_block = (struct heap_block*)((uint8_t*)block + HEAP_HEADER_SIZE + size);

	new_block->size = block->size - size - HEAP_HEADER_SIZE;

	new_block->free = 1;
	new_block->next = block->next;

	block->size = size;
	block->next = new_block;
}

void* kmalloc(size_t size) {
	if (size == 0) {
		return 0;
	}
	if (size > SIZE_MAX - (HEAP_ALIGNMENT - 1)) {
		return 0;
	}

	size = heap_align_size(size);

	struct heap_block* current = heap_head;

	while (current) {
		if (current->free && current->size >= size) {
			split_block(current, size);

			current->free = 0;

			return (uint8_t*)current + HEAP_HEADER_SIZE;
		}

		current = current->next;
	}

	if (heap_expand(size) != 0) {
		return 0;
	}

	return kmalloc(size);
}

static void merge_blocks(void) {
	struct heap_block* current = heap_head;

	while (current && current->next) {
		if (current->free && current->next->free) {
			current->size += HEAP_HEADER_SIZE + current->next->size;

			current->next = current->next->next;
		}
		else {
			current = current->next;
		}
	}
}

void kfree(void* ptr) {
	if (!ptr) {
		return;
	}

	struct heap_block* block = (struct heap_block*)((uint8_t*)ptr - HEAP_HEADER_SIZE);

	block->free = 1;

	merge_blocks();
}
