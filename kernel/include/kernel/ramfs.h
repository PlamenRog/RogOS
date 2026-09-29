#ifndef KERNEL_RAMFS_H
#define KERNEL_RAMFS_H

#include <kernel/vfs.h>

int ramfs_initialize(void);

struct vfs_node* ramfs_root(void);

#endif
