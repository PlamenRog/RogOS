#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH_SIZE 128
#define MAX_LINE_SIZE 256
#define MAX_FILE_SIZE 4096

static int read_line(char* buffer, uint32_t capacity) {
	if (!buffer || capacity == 0) {
		return -1;
	}

	uint32_t length = 0;

	for (;;) {
		int c = getchar();

		if (c == EOF) {
			return -1;
		}

		if (c == '\n') {
			putchar('\n');
			break;
		}

		// backspace to delete previous character
		if (c == '\b') {
			if (length > 0) {
				length--;

				write(1, "\b", 1); // move cursor left, erase, move cursor left
			}

			continue;
		}

		if (length >= capacity - 1) {
			continue;
		}

		buffer[length++] = (char)c;

		putchar(c);
	}

	buffer[length] = '\0';

	return (int)length;
}

static int load_file(const char* path, char* buffer, uint32_t capacity) {
	int fd = open(path, O_RDONLY);

	if (fd < 0) {
		return 0;
	}

	uint32_t total = 0;

	while (total < capacity) {
		int count = read(fd, buffer + total, capacity - total);

		if (count < 0) {
			close(fd);
			return -1;
		}

		if (count == 0) {
			break;
		}

		total += (uint32_t)count;
	}

	close(fd);

	return (int)total;
}

static int save_file(const char* path, const char* buffer, uint32_t length) {
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);

	if (fd < 0) {
		printf("Open failed\n");
		return -1;
	}

	int written = write(fd, buffer, length);

	close(fd);

	if (written != (int)length) {
		return -1;
	}

	return 0;
}

static void show_buffer(const char* buffer, uint32_t length) {
	printf("\n----- document -----\n");

	if (length == 0) {
		printf("[empty]\n");
	}
	else {
		write(1, buffer, length);

		if (buffer[length - 1] != '\n') {
			putchar('\n');
		}
	}

	printf("--------------------\n");
}

static void delete_last_line(char* buffer, uint32_t* length) {
	if (!buffer || !length) {
		return;
	}

	if (*length == 0) {
		return;
	}

	uint32_t pos = *length;

	if (pos > 0 && buffer[pos - 1] == '\n') {
		pos--;
	}

	while (pos > 0) {
		if (buffer[pos - 1] == '\n') {
			break;
		}

		pos--;
	}

	*length = pos;
}

static int append_line(char* buffer, uint32_t* length, uint32_t capacity, const char* line, uint32_t line_length) {
	if (*length + line_length + 1 > capacity) {
		return -1;
	}

	memcpy(buffer + *length, line, line_length);

	*length += line_length;

	buffer[(*length)++] = '\n';

	return 0;
}

static void print_help(void) {
	printf("\nCommands:\n"
		   "  .show   show current document\n"
		   "  .del    delete last line\n"
		   "  .clear  clear document\n"
		   "  .save   save and exit\n"
		   "  .quit   exit without saving\n"
		   "  .help   show this help\n"
		   "\n");
}

int main(void) {
	char path[MAX_PATH_SIZE];

	printf("RogOS Editor\n");
	printf("============\n");
	printf("File: ");

	int path_length = read_line(path, sizeof(path));

	if (path_length <= 0) {
		printf("Invalid filename\n");
		return 1;
	}

	char file_buffer[MAX_FILE_SIZE];

	int loaded = load_file(path, file_buffer, sizeof(file_buffer));

	if (loaded < 0) {
		printf("Failed to read file\n");
		return 1;
	}

	uint32_t file_length = (uint32_t)loaded;

	if (file_length == 0) {
		printf("\n[new file or empty file]\n");
	}
	else {
		show_buffer(file_buffer, file_length);
	}

	print_help();

	for (;;) {
		char line[MAX_LINE_SIZE];

		printf("> ");

		int line_length = read_line(line, sizeof(line));

		if (line_length < 0) {
			printf("Input error\n");
			return 1;
		}

		if (strcmp(line, ".save") == 0) {
			if (save_file(path, file_buffer, file_length) != 0) {
				printf("Failed to save file\n");
				return 1;
			}

			return 0;
		}

		if (strcmp(line, ".quit") == 0) {
			printf("Changes discarded\n");
			return 0;
		}

		if (strcmp(line, ".show") == 0) {
			show_buffer(file_buffer, file_length);

			continue;
		}

		if (strcmp(line, ".clear") == 0) {
			file_length = 0;

			printf("Document cleared\n");

			continue;
		}

		if (strcmp(line, ".del") == 0) {
			delete_last_line(file_buffer, &file_length);

			continue;
		}

		if (strcmp(line, ".help") == 0) {
			print_help();
			continue;
		}

		if (append_line(file_buffer, &file_length, sizeof(file_buffer), line, (uint32_t)line_length) != 0) {
			printf("Document is full (%u bytes max)\n", (unsigned int)sizeof(file_buffer));
		}
	}
}
