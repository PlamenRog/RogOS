// TODO: move shell impl into seperate repo: https://github.com/PlamenRog/RogShell
#include <kernel/keyboard.h>
#include <kernel/shell.h>
#include <kernel/tty.h>

#include <kernel/paging.h>
#include <kernel/pmm.h>

#define SHELL_BUFFER_SIZE 128

static char shell_buffer[SHELL_BUFFER_SIZE];

void shell_initialize(void) {
	terminal_writestring("RogOS shell\n");
}

static void shell_prompt(void) {
	terminal_writestring("> ");
}

static void shell_readline(void) {
	int index = 0;

	while (1) {
		char c = keyboard_getchar();

		if (c == '\n') {
			shell_buffer[index] = '\0';
			terminal_putchar('\n');
			return;
		}

		if (c == '\b') {
			if (index > 0) {
				index--;

				terminal_backspace();
			}

			continue;
		}

		if (index < SHELL_BUFFER_SIZE - 1) {
			shell_buffer[index++] = c;
			terminal_putchar(c);
		}
	}
}

void shell_run(void) {
	while (1) {
		shell_prompt();
		shell_readline();
		shell_execute(shell_buffer);
	}
}
