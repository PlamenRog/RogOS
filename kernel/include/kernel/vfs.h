#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#include <stdint.h>

#define VFS_MAX_NAME 128

#define VFS_FILE 0x01
#define VFS_DIR 0x02

struct vfs_node;
struct vfs_filesystem;
struct vfs_iterator;
struct vfs_dirent;

typedef uint32_t (*vfs_read_t)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
typedef uint32_t (*vfs_write_t)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);

typedef struct vfs_node* (*vfs_create_t)(struct vfs_node* parent, const char* name);

typedef int (*vfs_open_dir_t)(struct vfs_node* node, struct vfs_iterator* iterator);
typedef int (*vfs_read_dir_t)(struct vfs_iterator* iterator, struct vfs_dirent* entry);

int vfs_truncate(struct vfs_node* node);

typedef int (*vfs_truncate_t)(struct vfs_node* node);

struct vfs_node* vfs_create(const char* path);

struct vfs_filesystem {
	vfs_read_t read;
	vfs_write_t write;
	vfs_create_t create;
	vfs_truncate_t truncate;

	vfs_open_dir_t open_dir;
	vfs_read_dir_t read_dir;
};

struct vfs_node {
	char name[VFS_MAX_NAME];

	uint32_t size;
	uint32_t flags;

	void* private_data;

	struct vfs_filesystem* filesystem;

	struct vfs_node* child;
	struct vfs_node* next;
};

struct vfs_dirent {
	char name[VFS_MAX_NAME];

	uint32_t flags;
	uint32_t size;
};

struct vfs_iterator {
	struct vfs_node* node;
	void* private_data;
};

void vfs_mount(struct vfs_node* node);

struct vfs_node* vfs_get_root(void);
struct vfs_node* vfs_find(const char* path);

uint32_t vfs_read(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t vfs_write(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);

int vfs_open_dir(struct vfs_node* node, struct vfs_iterator* iterator);
int vfs_read_dir(struct vfs_iterator* iterator, struct vfs_dirent* entry);

#endif
