#include <kernel/vfs.h>

#include <string.h>

static struct vfs_node* root = 0;

static struct vfs_node* find_child(struct vfs_node* parent, const char* name) {
	if (!parent || !name) {
		return 0;
	}

	struct vfs_node* current = parent->child;

	while (current) {
		if (strcmp(current->name, name) == 0) {
			return current;
		}

		current = current->next;
	}

	return 0;
}

void vfs_mount(struct vfs_node* node) {
	root = node;
}

struct vfs_node* vfs_create(const char* path) {
	if (!root || !path) {
		return 0;
	}

	while (*path == '/') {
		path++;
	}

	if (!*path) {
		return 0;
	}

	struct vfs_node* current = root;

	char part[VFS_MAX_NAME];

	while (*path) {
		int length = 0;

		while (*path && *path != '/') {
			if (length >= VFS_MAX_NAME - 1) {
				return 0;
			}

			part[length++] = *path++;
		}

		part[length] = '\0';
		while (*path == '/') {
			path++;
		}

		if (!*path) {
			if (!(current->flags & VFS_DIR)) {
				return 0;
			}

			// dont create duplicates
			if (find_child(current, part)) {
				return 0;
			}

			if (!current->filesystem || !current->filesystem->create) {
				return 0;
			}

			return current->filesystem->create(current, part);
		}

		current = find_child(current, part);

		if (!current || !(current->flags & VFS_DIR)) {
			return 0;
		}
	}

	return 0;
}

struct vfs_node* vfs_get_root(void) {
	return root;
}

struct vfs_node* vfs_find(const char* path) {
	if (!root || !path) {
		return 0;
	}

	while (*path == '/') {
		path++;
	}

	if (!*path) {
		return root;
	}

	struct vfs_node* current = root;

	char part[VFS_MAX_NAME];

	while (*path) {
		while (*path == '/') {
			path++;
		}

		if (*path == '\0') {
			break;
		}

		uint32_t length = 0;

		while (*path && *path != '/') {
			if (length >= VFS_MAX_NAME - 1) {
				return 0;
			}

			part[length++] = *path++;
		}

		part[length] = '\0';

		current = find_child(current, part);

		if (!current) {
			return 0;
		}
	}

	return current;
}

uint32_t vfs_read(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	if (!node || !buffer || !node->filesystem || !node->filesystem->read) {
		return 0;
	}

	return node->filesystem->read(node, offset, size, buffer);
}

uint32_t vfs_write(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	if (!node || !buffer || !node->filesystem || !node->filesystem->write) {
		return 0;
	}

	return node->filesystem->write(node, offset, size, buffer);
}

int vfs_open_dir(struct vfs_node* node, struct vfs_iterator* iterator) {
	if (!node || !iterator || !node->filesystem || !node->filesystem->open_dir) {
		return -1;
	}

	return node->filesystem->open_dir(node, iterator);
}

int vfs_read_dir(struct vfs_iterator* iterator, struct vfs_dirent* entry) {
	if (!iterator || !entry || !iterator->node || !iterator->node->filesystem || !iterator->node->filesystem->read_dir) {
		return -1;
	}

	return iterator->node->filesystem->read_dir(iterator, entry);
}

int vfs_truncate(struct vfs_node* node) {
	if (!node || !node->filesystem || !node->filesystem->truncate) {
		return -1;
	}

	return node->filesystem->truncate(node);
}
