#include <unistd.h>

int close(int fd) {
	int result;

	asm volatile("int $0x80" : "=a"(result) : "a"(3), "b"(fd) : "memory");

	return result;
}
