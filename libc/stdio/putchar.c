#include <stdio.h>

#if defined(__is_libk)

#include <kernel/tty.h>

#else

#include <unistd.h>

#endif

int putchar(int ic) {
	char c = (char)ic;

#if defined(__is_libk)

	terminal_write(&c, sizeof(c));

#else

	if (write(1, &c, sizeof(c)) != sizeof(c)) {
		return EOF;
	}

#endif

	return (unsigned char)c;
}
