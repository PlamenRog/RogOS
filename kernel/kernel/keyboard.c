#include <stdint.h>

#include <kernel/io.h>
#include <kernel/keyboard.h>

static char keyboard_buffer[128];

static volatile uint8_t buffer_read = 0;
static volatile uint8_t buffer_write = 0;

// using scancode set 1 for us qwerty: https://wiki.osdev.org/PS/2_Keyboard#Scan_Code_Set_1
static const char keyboard_map[128] = {
	0, // 0x00
	0, // 0x01 escape
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'\b', '\t', // backspace and tab
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
	'\n',
	0, // lctrl
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, // lshift
	'\\',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0, // rshift
	'*',
	0, // lalt
	' '
};

static int shift_pressed = 0;
static const char keyboard_shift_map[128] = {
	0, 0,
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	'\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
	'\n',
	0,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	0,
	'|',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'
};

static void keyboard_push(char c) {
	uint8_t next = (buffer_write + 1) % 128;

	if (next != buffer_read) {
		keyboard_buffer[buffer_write] = c;
		buffer_write = next;
	}
}

void keyboard_handler(void) {
	uint8_t scancode = inb(0x60);

	// lshift press
	if (scancode == 0x2A || scancode == 0x36) {
		shift_pressed = 1;
		return;
	}

	// lshift release
	if (scancode == 0xAA || scancode == 0xB6) {
		shift_pressed = 0;
		return;
	}

	// ignores other releases
	if (scancode & 0x80) {
		return;
	}

	char c;

	if (shift_pressed) {
		c = keyboard_shift_map[scancode];
	}
	else {
		c = keyboard_map[scancode];
	}

	if (c) {
		keyboard_push(c);
	}
}

char keyboard_getchar(void) {
	uint32_t saved_flags;

	__asm__ volatile("pushfl\n\tpopl %0\n\tcli" : "=r"(saved_flags) : : "memory");

	for (;;) {
		if (buffer_read != buffer_write) {
			char c = keyboard_buffer[buffer_read];

			buffer_read = (buffer_read + 1) % 128;

			if (saved_flags & (1u << 9)) {
				__asm__ volatile("sti" : : : "memory");
			}

			return c;
		}

		// irq1 resumes action after STI
		__asm__ volatile("sti\n\thlt\n\tcli" : : : "memory");
	}
}
