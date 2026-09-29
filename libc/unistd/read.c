#include <unistd.h>

ssize_t read(int fd, void* buffer, uint32_t count) {
	int result;

	asm volatile("int $0x80" : "=a"(result) : "a"(4), "b"(fd), "c"(buffer), "d"(count) : "memory");

	return result;
}
