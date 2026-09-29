// TODO: move shell impl into seperate repo: https://github.com/PlamenRog/RogShell
#include <kernel/exec.h>
#include <kernel/power.h>
#include <kernel/shell.h>
#include <kernel/tty.h>
#include <kernel/vfs.h>
#include <string.h>

#define MAX_ARGS 16

typedef void (*command_handler)(int argc, char** argv);

struct shell_command {
	const char* name;
	command_handler handler;
};

static void command_help(int argc, char** argv) {
	(void)argc;
	(void)argv;

	terminal_writestring("\nAvailable commands:\n");
	terminal_writestring(" help  - show this message\n");
	terminal_writestring(" clear - clear screen\n");
	terminal_writestring(" about - information about RogOS\n");
	terminal_writestring(" ls - list directory contents\n");
	terminal_writestring(" cat - show file contents\n");
	terminal_writestring(" shutdown - shuts down the OS\n");
}

static void command_about(int argc, char** argv) {
	(void)argc;
	(void)argv;

	terminal_writestring("\nRogOS\n");
	terminal_writestring("A hobby operating system\n");
	terminal_writestring("Architecture: i386\n");
}

static int node_is_elf(struct vfs_node* node) {
	if (!node) {
		return 0;
	}

	if (!(node->flags & VFS_FILE)) {
		return 0;
	}

	if (node->size < 4) {
		return 0;
	}

	uint8_t magic[4];

	if (vfs_read(node, 0, sizeof(magic), magic) != sizeof(magic)) {
		return 0;
	}

	return magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F';
}

static void command_ls(int argc, char** argv) {
	(void)argc;
	(void)argv;

	struct vfs_iterator iterator;
	struct vfs_dirent entry;

	if (vfs_open_dir(vfs_get_root(), &iterator)) {
		terminal_writestring("not a directory\n");

		return;
	}

	while (vfs_read_dir(&iterator, &entry)) {
		struct vfs_node* node = vfs_find(entry.name);

		if (node && node_is_elf(node)) {
			terminal_setcolor(0x0A); // light green fg + black bg
		}
		else {
			terminal_setcolor(0x07); // grey fg + black bg
		}

		terminal_writestring(entry.name);
		terminal_putchar('\n');
	}

	terminal_setcolor(0x07);
}

static int shell_parse(char* input, char** argv) {
	int argc = 0;

	while (*input) {
		while (*input == ' ') {
			input++;
		}

		if (*input == '\0') {
			break;
		}

		argv[argc++] = input;

		while (*input && *input != ' ') {
			input++;
		}

		if (*input) {
			*input = '\0';
			input++;
		}

		if (argc >= MAX_ARGS) {
			break;
		}
	}

	return argc;
}

static void command_cat(int argc, char** argv) {
	if (argc < 2) {
		terminal_writestring("usage: cat <file>\n");

		return;
	}

	struct vfs_node* node = vfs_find(argv[1]);

	if (!node) {
		terminal_writestring("file not found\n");

		return;
	}

	if (!(node->flags & VFS_FILE)) {
		terminal_writestring("not a file\n");

		return;
	}

	uint8_t buffer[256];
	uint32_t offset = 0;

	for (;;) {
		uint32_t count = vfs_read(node, offset, sizeof(buffer), buffer);

		if (count == 0) {
			break;
		}

		terminal_write((const char*)buffer, count);

		offset += count;
	}
}

static void command_clear(int argc, char** argv) {
	(void)argc;
	(void)argv;

	terminal_initialize();
}

static void command_shutdown(int argc, char** argv) {
	(void)argc;
	(void)argv;

	terminal_writestring("Shutting down...\n");

	power_shutdown();
}

static struct shell_command commands[] = {
	{ "help", command_help },
	{ "clear", command_clear },
	{ "about", command_about },
	{ "ls", command_ls },
	{ "shutdown", command_shutdown },
	{ "cat", command_cat },
	{ 0, 0 }
};

void shell_execute(char* input) {
	char* argv[MAX_ARGS];

	int argc = shell_parse(input, argv);

	if (argc == 0) {
		return;
	}

	for (int i = 0; commands[i].name; i++) {
		if (strcmp(argv[0], commands[i].name) == 0) {
			commands[i].handler(argc, argv);

			return;
		}
	}

	const char* program = argv[0];

	// ramfs is flat so  './' is out of habit
	if (program[0] == '.' && program[1] == '/') {
		program += 2;
	}

	if (exec_program(program) == 0) {
		return;
	}

	terminal_writestring("Unknown command\n");
}
