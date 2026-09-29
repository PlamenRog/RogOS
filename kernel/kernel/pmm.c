#include <kernel/multiboot.h>
#include <kernel/pmm.h>

#include <stdint.h>

#define FRAME_SIZE 0x1000u
#define MAX_MEMORY (128u * 1024u * 1024u) // limits the ram managed by os

#define FRAME_COUNT (MAX_MEMORY / FRAME_SIZE)

#define PMM_LOW_RESERVED_LIMIT 0x04000000u

extern uint32_t kernel_end;

static uint8_t bitmap[FRAME_COUNT / 8];

static void bitmap_set(uint32_t frame) {
	uint32_t byte_index = frame / 8;

	uint32_t bit_index = frame % 8;

	uint8_t bit_mask = (uint8_t)(1u << bit_index);

	bitmap[byte_index] |= bit_mask;
}

static void bitmap_clear(uint32_t frame) {
	uint32_t byte_index = frame / 8;

	uint32_t bit_index = frame % 8;

	uint8_t bit_mask = (uint8_t)(1u << bit_index);

	uint8_t clear_mask = (uint8_t)~bit_mask;

	bitmap[byte_index] &= clear_mask;
}

static int bitmap_test(uint32_t frame) {
	uint32_t byte_index = frame / 8;
	uint32_t bit_index = frame % 8;

	uint8_t bit_mask = (uint8_t)(1u << bit_index);

	uint8_t result = bitmap[byte_index] & bit_mask;

	return result != 0;
}

static uint32_t align_up_frame(uint32_t address) {
	uint32_t offset_mask = FRAME_SIZE - 1;
	uint32_t adjusted_address = address + offset_mask;
	uint32_t frame_mask = ~offset_mask;
	uint32_t aligned_address = adjusted_address & frame_mask;

	return aligned_address;
}

static uint32_t align_down_frame(uint32_t address) {
	uint32_t offset_mask = FRAME_SIZE - 1;
	uint32_t frame_mask = ~offset_mask;
	uint32_t aligned_address = address & frame_mask;

	return aligned_address;
}

static void mark_region_available(uint64_t base, uint64_t length) {
	if (length == 0) {
		return;
	}

	uint64_t end;
	if (length > UINT64_MAX - base) {
		end = UINT64_MAX;
	}
	else {
		end = base + length;
	}

	if (base >= MAX_MEMORY) {
		return;
	}

	if (end > MAX_MEMORY) {
		end = MAX_MEMORY;
	}

	uint32_t start32 = (uint32_t)base;
	uint32_t end32 = (uint32_t)end;

	uint32_t start = align_up_frame(start32);
	uint32_t finish = align_down_frame(end32);

	if (start >= finish) {
		return;
	}

	for (uint32_t address = start; address < finish; address += FRAME_SIZE) {
		bitmap_clear(address / FRAME_SIZE);
	}
}

void pmm_initialize(uint32_t multiboot_addr) {
	for (uint32_t i = 0; i < sizeof(bitmap); i++) {
		bitmap[i] = 0xFF;
	}

	struct multiboot_info* mb = (struct multiboot_info*)multiboot_addr;

	if (!(mb->flags & MULTIBOOT_INFO_MEMORY_MAP)) {
		return;
	}
	if (mb->mmap_addr > UINT32_MAX - mb->mmap_length) {
		return;
	}

	uint32_t mmap_end = mb->mmap_addr + mb->mmap_length;

	uint32_t current = mb->mmap_addr;

	while (current < mmap_end) {
		if (mmap_end - current < sizeof(uint32_t)) {
			break;
		}

		struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*)current;

		// multiboot size shouldnt include size field
		uint32_t entry_size = entry->size + sizeof(entry->size);

		if (entry_size < sizeof(struct multiboot_mmap_entry)) {
			break;
		}
		if (entry_size > mmap_end - current) {
			break;
		}

		if (entry->type == 1) {
			mark_region_available(entry->addr, entry->len);
		}
		current += entry_size;
	}

	// reserves first 1mib
	for (uint32_t address = 0; address < 0x100000u; address += FRAME_SIZE) {
		bitmap_set(address / FRAME_SIZE);
	}

	// reserves kernel image
	uint32_t kernel_start = 0x100000u;
	uint32_t kernel_finish = align_up_frame((uint32_t)&kernel_end);

	if (kernel_finish > MAX_MEMORY) {
		kernel_finish = MAX_MEMORY;
	}

	for (uint32_t address = kernel_start; address < kernel_finish; address += FRAME_SIZE) {
		bitmap_set(address / FRAME_SIZE);
	}
}

uint32_t pmm_alloc_frame_below(uint32_t limit) {
	if (limit > MAX_MEMORY) {
		limit = MAX_MEMORY;
	}

	uint32_t frame_limit = limit / FRAME_SIZE;

	for (uint32_t frame = 0; frame < frame_limit; frame++) {
		if (!bitmap_test(frame)) {
			bitmap_set(frame);
			return frame * FRAME_SIZE;
		}
	}

	return 0;
}

uint32_t pmm_alloc_frame(void) {
	uint32_t preferred_start = PMM_LOW_RESERVED_LIMIT / FRAME_SIZE;

	// prefer frames >=64 mib first
	for (uint32_t frame = preferred_start; frame < FRAME_COUNT; frame++) {
		if (!bitmap_test(frame)) {
			bitmap_set(frame);

			return frame * FRAME_SIZE;
		}
	}

	// fallback to low memory frame
	for (uint32_t frame = 0; frame < preferred_start && frame < FRAME_COUNT; frame++) {
		if (!bitmap_test(frame)) {
			bitmap_set(frame);

			return frame * FRAME_SIZE;
		}
	}

	return 0;
}

void pmm_free_frame(uint32_t address) {
	if (address & (FRAME_SIZE - 1)) {
		return;
	}
	if (address >= MAX_MEMORY) {
		return;
	}
	if (address < 0x100000u) {
		return;
	}

	bitmap_clear(address / FRAME_SIZE);
}
