#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(__is_libk)

#include <kernel/tty.h>

#else

#include <unistd.h>

#endif

static bool print(const char* data, size_t length) {
	if (length == 0) {
		return true;
	}

#if defined(__is_libk)

	terminal_write(data, length);

	return true;

#else

	return write(1, data, (uint32_t)length) == (ssize_t)length;

#endif
}

static bool print_char(char c, int* written) {
	if (putchar(c) == EOF) {
		return false;
	}

	(*written)++;
	return true;
}

static bool print_unsigned(unsigned int value, unsigned int base, bool uppercase, int width, char padding, int* written) {
	char buffer[32];
	const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

	size_t length = 0;

	do {
		buffer[length++] = digits[value % base];
		value /= base;
	}
	while (value);

	while ((int)length < width) {
		if (!print_char(padding, written)) {
			return false;
		}
		width--;
	}

	while (length > 0) {
		if (!print_char(buffer[--length], written)) {
			return false;
		}
	}

	return true;
}

static bool print_signed(int value, int width, char padding, int* written) {
	bool negative = value < 0;

	unsigned int magnitude;

	if (negative) {
		magnitude = 0u - (unsigned int)value;

		if (!print_char('-', written)) {
			return false;
		}

		if (width > 0) {
			width--;
		}
	}
	else {
		magnitude = (unsigned int)value;
	}

	return print_unsigned(magnitude, 10, false, width, padding, written);
}

int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		if (*format != '%') {
			const char* start = format;

			while (*format != '\0' && *format != '%') {
				format++;
			}

			size_t length = (size_t)(format - start);

			if (!print(start, length)) {
				goto fail;
			}

			written += (int)length;

			continue;
		}

		format++;

		if (*format == '\0') {
			if (!print_char('%', &written)) {
				goto fail;
			}

			break;
		}

		if (*format == '%') {
			if (!print_char('%', &written)) {
				goto fail;
			}

			format++;
			continue;
		}

		char padding = ' ';
		int width = 0;

		if (*format == '0') {
			padding = '0';
			format++;
		}

		while (*format >= '0' && *format <= '9') {
			int digit = *format - '0';

			if (width > (INT_MAX - digit) / 10) {
				width = INT_MAX;
			}
			else {
				width = width * 10 + digit;
			}

			format++;
		}

		if (*format == '\0') {
			if (!print_char('%', &written)) {
				goto fail;
			}

			break;
		}

		switch (*format) {
		case 'c': {
			char c = (char)va_arg(parameters, int);

			if (!print_char(c, &written)) {
				goto fail;
			}

			break;
		}

		case 's': {
			const char* str = va_arg(parameters, const char*);

			size_t length = strlen(str);

			if (!print(str, length)) {
				goto fail;
			}

			written += (int)length;

			break;
		}

		case 'd':
		case 'i': {
			int value = va_arg(parameters, int);

			if (!print_signed(value, width, padding, &written)) {
				goto fail;
			}

			break;
		}

		case 'u': {
			unsigned int value = va_arg(parameters, unsigned int);

			if (!print_unsigned(value, 10, false, width, padding, &written)) {
				goto fail;
			}

			break;
		}

		case 'x':
		case 'X': {
			unsigned int value = va_arg(parameters, unsigned int);

			bool uppercase = (*format == 'X');

			if (!print_unsigned(value, 16, uppercase, width, padding, &written)) {
				goto fail;
			}

			break;
		}

		case 'p': {
			uintptr_t value = (uintptr_t)va_arg(parameters, void*);

			if (!print("0x", 2)) {
				goto fail;
			}

			written += 2;

			if (!print_unsigned((unsigned int)value, 16, false, sizeof(uintptr_t) * 2, '0',
								&written)) {
				goto fail;
			}

			break;
		}

		default: {
			// prints literally when unknown
			if (!print_char('%', &written)) {
				goto fail;
			}

			if (!print_char(*format, &written)) {
				goto fail;
			}

			break;
		}
		}

		format++;
	}

	va_end(parameters);

	return written;

fail:
	va_end(parameters);

	return -1;
}
