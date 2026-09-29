#include <stdio.h>

#if defined(__is_libk)

#include <kernel/keyboard.h>

#else

#include <unistd.h>

#endif

int getchar(void) {
#if defined(__is_libk)

	return (unsigned char)keyboard_getchar();

#else

	char c;

	if (read(0, &c, 1) != 1) {
		return EOF;
	}

	return (unsigned char)c;

#endif
}
