#include <fcntl.h>

int open(const char* path, int flags) {
	int result;

	asm volatile("int $0x80" : "=a"(result) : "a"(2), "b"(path), "c"(flags) : "memory");

	return result;
}
