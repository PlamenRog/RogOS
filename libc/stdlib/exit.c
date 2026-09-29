#include <stdlib.h>

__attribute__((noreturn)) void exit(int status) {
	__asm__ volatile("int $0x80" : : "a"(0), "b"(status) : "memory");

	for (;;) {
	}
}
