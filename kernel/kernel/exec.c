#include <kernel/elf_loader.h>
#include <kernel/exec.h>
#include <kernel/process.h>

int exec_program(const char* path) {
	struct process* process = process_create(path);

	if (!process) {
		return -1;
	}

	if (elf_load(process, path) != 0) {
		process_destroy(process);
		return -1;
	}

	process_start(process);

	process_destroy(process);

	return 0;
}
