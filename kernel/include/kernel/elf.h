#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#define ELF_MAGIC 0x464C457F

#define ELF_PF_X 0x1
#define ELF_PF_W 0x2
#define ELF_PF_R 0x4

#define ELF_PT_LOAD 1

#define ELF_CLASS_32 1
#define ELF_DATA_LSB 1
#define ELF_VERSION_CURRENT 1

#define ELF_ET_EXEC 2
#define ELF_EM_386 3

struct elf_header {
	uint32_t magic;
	uint8_t ident[12];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
} __attribute__((packed));

struct elf_program_header {
	uint32_t type;
	uint32_t offset;
	uint32_t virtual_address;
	uint32_t physical_address;
	uint32_t file_size;
	uint32_t memory_size;
	uint32_t flags;
	uint32_t align;
} __attribute__((packed));

#endif
