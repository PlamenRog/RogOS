#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/io.h>
#include <kernel/tty.h>

#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

static bool terminal_batch_write = false;
static int terminal_escape_state = 0;
static uint32_t terminal_escape_value = 0;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

static void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

static void terminal_update_cursor(void) {
	if (terminal_batch_write) {
		return;
	}

	uint16_t position = terminal_row * VGA_WIDTH + terminal_column;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(position & 0xFF));

	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

void terminal_backspace(void) {
	if (terminal_column == 0) {
		return;
	}

	terminal_column--;

	terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);

	terminal_update_cursor();
}

static void terminal_scroll(void) {
	// moves every row up
	for (size_t y = 1; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			terminal_buffer[(y - 1) * VGA_WIDTH + x] = terminal_buffer[y * VGA_WIDTH + x];
		}
	}

	// clears the last row
	for (size_t x = 0; x < VGA_WIDTH; x++) {
		terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
	}

	terminal_row = VGA_HEIGHT - 1;
	terminal_column = 0;
}

static void terminal_handle_escape(char c) {
	// state 1 - expects '['
	if (terminal_escape_state == 1) {
		if (c == '[') {
			terminal_escape_state = 2;
			terminal_escape_value = 0;
		}
		else {
			terminal_escape_state = 0;
		}

		return;
	}

	// state 2 - CSI sequence
	if (terminal_escape_state == 2) {
		if (c >= '0' && c <= '9') {
			uint32_t digit = (uint32_t)(c - '0');

			// clamps instead of handling integer overflow
			if (terminal_escape_value > (VGA_HEIGHT - digit) / 10) {
				terminal_escape_value = VGA_HEIGHT;
			}
			else {
				terminal_escape_value = terminal_escape_value * 10 + digit;
			}

			return;
		}

		// ESC [ n A -> moves cursor up rows
		if (c == 'A') {
			uint32_t rows = terminal_escape_value != 0 ? terminal_escape_value : 1;

			if (rows > terminal_row) {
				terminal_row = 0;
			}
			else {
				terminal_row -= rows;
			}

			terminal_escape_state = 0;

			terminal_update_cursor();
			return;
		}

		// ESC [ H -> moves cursor to top left corner
		if (c == 'H') {
			terminal_row = 0;
			terminal_column = 0;

			terminal_escape_state = 0;

			terminal_update_cursor();
			return;
		}

		// ESC [ 2 J -> clears the screen
		if (c == 'J' && terminal_escape_value == 2) {
			for (size_t y = 0; y < VGA_HEIGHT; y++) {
				for (size_t x = 0; x < VGA_WIDTH; x++) {
					terminal_putentryat(' ', terminal_color, x, y);
				}
			}

			terminal_row = 0;
			terminal_column = 0;

			terminal_escape_state = 0;

			terminal_update_cursor();
			return;
		}

		// when sequence isnt supported
		terminal_escape_state = 0;
	}
}

void terminal_putchar(char c) {
	// ansi escape
	if (c == '\x1B') {
		terminal_escape_state = 1;
		terminal_escape_value = 0;
		return;
	}

	if (terminal_escape_state != 0) {
		terminal_handle_escape(c);
		return;
	}

	switch (c) {
	case '\n':
		terminal_column = 0;
		terminal_row++;

		if (terminal_row == VGA_HEIGHT) {
			terminal_scroll();
		}

		terminal_update_cursor();
		return;

	case '\r':
		terminal_column = 0;
		terminal_update_cursor();
		return;

	case '\b':
		terminal_backspace();
		return;

	default:
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);

		terminal_column++;

		if (terminal_column == VGA_WIDTH) {
			terminal_column = 0;
			terminal_row++;

			if (terminal_row == VGA_HEIGHT) {
				terminal_scroll();
			}
		}

		terminal_update_cursor();
		return;
	}
}

void terminal_write(const char* data, size_t size) {
	terminal_batch_write = true;

	for (size_t i = 0; i < size; i++) {
		terminal_putchar(data[i]);
	}

	terminal_batch_write = false;

	terminal_update_cursor();
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
