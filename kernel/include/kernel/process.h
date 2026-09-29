#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define PROCESS_NAME_MAX 32
#define PROCESS_MAX_FDS 32

#define REGION_READ 1
#define REGION_WRITE 2
#define REGION_EXEC 4

#define FD_FLAG_READ 1
#define FD_FLAG_WRITE 2

struct page_directory;
struct vfs_node;

struct cpu_context {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;

	uint32_t eip;
	uint32_t eflags;
};

struct process_fd {
	struct vfs_node* node;

	uint32_t offset;
	uint32_t flags;

	uint8_t used;
};

struct process {
	uint32_t pid;
	char name[PROCESS_NAME_MAX];

	struct page_directory* page_directory;

	struct cpu_context context;
	struct cpu_context parent_context;

	uint32_t entry;
	uint32_t kernel_stack;
	uint32_t user_stack;

	struct process_fd fds[PROCESS_MAX_FDS];
};

struct process* process_create(const char* name);
struct process* process_get_current(void);

int process_allocate_page(struct process* process, uint32_t virtual_address, uint32_t region_flags);

int process_map_region(struct process* process, uint32_t address, uint32_t size, uint32_t flags);

struct process_fd* process_fd_get(struct process* process, int fd);

int process_fd_allocate(struct process* process, struct vfs_node* node, uint32_t flags);
int process_fd_close(struct process* process, int fd);
int process_set_region_flags(struct process* process, uint32_t address, uint32_t size, uint32_t region_flags);

void process_start(struct process* process);
void process_exit(int status) __attribute__((noreturn));
void process_destroy(struct process* process);

#endif
