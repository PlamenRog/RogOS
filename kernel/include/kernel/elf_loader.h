#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <kernel/process.h>

#define USER_PROGRAM_MIN 0x08000000u

#define USER_STACK_TOP 0x80000000u
#define USER_STACK_SIZE 0x4000u
#define USER_STACK_BOTTOM (USER_STACK_TOP - USER_STACK_SIZE)

int elf_load(struct process* process, const char* path);

#endif
