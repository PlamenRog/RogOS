// TODO: add persistant ext2/3: https://wiki.osdev.org/File_Systems
#include <kernel/heap.h>
#include <kernel/initramfs.h>
#include <kernel/ramfs.h>

#include <string.h>

static struct vfs_node* root;

static uint32_t read_u32(const uint8_t** data) {
	const uint8_t* current = *data;

	uint32_t byte0 = (uint32_t)current[0];

	uint32_t byte1 = (uint32_t)current[1] << 8;

	uint32_t byte2 = (uint32_t)current[2] << 16;

	uint32_t byte3 = (uint32_t)current[3] << 24;

	uint32_t value = byte0 | byte1 | byte2 | byte3;

	*data = current + 4;

	return value;
}

static void ramfs_add_child(struct vfs_node* parent, struct vfs_node* child) {
	if (!parent || !child) {
		return;
	}

	child->next = parent->child;

	parent->child = child;
}

static struct vfs_node* ramfs_create_node(const char* name, const void* data, uint32_t size, struct vfs_filesystem* filesystem) {
	if (!name || !filesystem) {
		return 0;
	}

	struct vfs_node* node = kmalloc(sizeof(struct vfs_node));

	if (!node) {
		return 0;
	}

	memset(node, 0, sizeof(struct vfs_node));

	strncpy(node->name, name, VFS_MAX_NAME - 1);

	node->name[VFS_MAX_NAME - 1] = '\0';
	node->flags = VFS_FILE;
	node->filesystem = filesystem;
	node->size = size;

	if (size > 0) {
		if (!data) {
			kfree(node);
			return 0;
		}

		node->private_data = kmalloc(size);

		if (!node->private_data) {
			kfree(node);
			return 0;
		}

		memcpy(node->private_data, data, size);
	}

	return node;
}

static uint32_t ramfs_read(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	if (!node || !buffer) {
		return 0;
	}
	if (!(node->flags & VFS_FILE)) {
		return 0;
	}
	if (offset >= node->size) {
		return 0;
	}

	uint32_t remaining = node->size - offset;

	if (size > remaining) {
		size = remaining;
	}

	memcpy(buffer, (uint8_t*)node->private_data + offset, size);

	return size;
}

static uint32_t ramfs_write(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
	if (!node || !buffer || size == 0) {
		return 0;
	}
	if (!(node->flags & VFS_FILE)) {
		return 0;
	}

	if (offset > UINT32_MAX - size) {
		return 0;
	}

	uint32_t required = offset + size;

	if (required > node->size) {
		uint8_t* new_data = kmalloc(required);

		if (!new_data) {
			return 0;
		}

		memset(new_data, 0, required);

		if (node->private_data && node->size > 0) {
			memcpy(new_data, node->private_data, node->size);

			kfree(node->private_data);
		}

		node->private_data = new_data;

		node->size = required;
	}

	memcpy((uint8_t*)node->private_data + offset, buffer, size);

	return size;
}

static int ramfs_truncate(struct vfs_node* node) {
	if (!node) {
		return -1;
	}
	if (!(node->flags & VFS_FILE)) {
		return -1;
	}

	if (node->private_data) {
		kfree(node->private_data);

		node->private_data = 0;
	}

	node->size = 0;

	return 0;
}

static int ramfs_open_dir(struct vfs_node* node, struct vfs_iterator* iterator) {
	if (!node || !iterator) {
		return -1;
	}
	if (!(node->flags & VFS_DIR)) {
		return -1;
	}

	iterator->node = node;

	iterator->private_data = node->child;

	return 0;
}

static int ramfs_read_dir(struct vfs_iterator* iterator, struct vfs_dirent* entry) {
	if (!iterator || !entry) {
		return 0;
	}

	struct vfs_node* current = iterator->private_data;

	if (!current) {
		return 0;
	}

	strncpy(entry->name, current->name, VFS_MAX_NAME - 1);

	entry->name[VFS_MAX_NAME - 1] = '\0';
	entry->flags = current->flags;
	entry->size = current->size;

	iterator->private_data = current->next;

	return 1;
}

static struct vfs_node* ramfs_create(struct vfs_node* parent, const char* name) {
	if (!parent || !name) {
		return 0;
	}
	if (!(parent->flags & VFS_DIR)) {
		return 0;
	}

	// new node gets inserted as filesystem parent
	struct vfs_node* node = ramfs_create_node(name, 0, 0, parent->filesystem);

	if (!node) {
		return 0;
	}

	ramfs_add_child(parent, node);

	return node;
}

static struct vfs_filesystem ramfs = {
	.read = ramfs_read,
	.write = ramfs_write,
	.create = ramfs_create,
	.truncate = ramfs_truncate,
	.open_dir = ramfs_open_dir,
	.read_dir = ramfs_read_dir
};

static void ramfs_load_initramfs(void) {
	const uint8_t* data = INITRAMFS_START;
	const uint8_t* end = INITRAMFS_START + INITRAMFS_SIZE;

	if (end - data < 8) {
		return;
	}
	if (data[0] != 'R' || data[1] != 'F' || data[2] != 'S' || data[3] != '1') {
		return;
	}

	data += 4;

	uint32_t count = read_u32(&data);

	for (uint32_t i = 0; i < count; i++) {
		if (end - data < 8) {
			return;
		}

		uint32_t name_length = read_u32(&data);
		uint32_t file_size = read_u32(&data);

		if (name_length == 0 || name_length >= VFS_MAX_NAME) {
			return;
		}

		uint32_t remaining = (uint32_t)(end - data);

		if (name_length > remaining) {
			return;
		}

		remaining -= name_length;

		if (file_size > remaining) {
			return;
		}

		char name[VFS_MAX_NAME];

		memcpy(name, data, name_length);

		name[name_length] = '\0';

		data += name_length;

		struct vfs_node* node = ramfs_create_node(name, data, file_size, &ramfs);

		if (node) {
			ramfs_add_child(root, node);
		}

		data += file_size;
	}
}

int ramfs_initialize(void) {
	root = kmalloc(sizeof(struct vfs_node));

	if (!root) {
		return -1;
	}

	memset(root, 0, sizeof(struct vfs_node));

	root->flags = VFS_DIR;
	root->filesystem = &ramfs;
	root->name[0] = '/';
	root->name[1] = '\0';

	// not necessary for os long term but nice for showcasing
	static const char welcome_text[] = "Hello from RogOS!\n";

	struct vfs_node* welcome = ramfs_create_node("welcome.txt", welcome_text, sizeof(welcome_text) - 1, &ramfs);

	if (welcome) {
		ramfs_add_child(root, welcome);
	}

	ramfs_load_initramfs();

	return 0;
}

struct vfs_node* ramfs_root(void) {
	return root;
}
