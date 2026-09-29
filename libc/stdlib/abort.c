#include <stdio.h>
#include <stdlib.h>

__attribute__((noreturn)) void abort(void) {
#if defined(__is_libk)

	printf("kernel: panic: abort()\n");

	for (;;) {
		__asm__ volatile("cli; hlt");
	}

#else

	printf("abort()\n");

	exit(1);

#endif

	__builtin_unreachable();
}
