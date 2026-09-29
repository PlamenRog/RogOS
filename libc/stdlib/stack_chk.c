#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uintptr_t __stack_chk_guard = 0xDEADBEEF;

__attribute__((noreturn)) void __stack_chk_fail(void) {
#if defined(__is_libk)
	printf("kernel: stack smashing detected\n");
#else
	printf("stack smashing detected\n");
#endif

	abort();
}
