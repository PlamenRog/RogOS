#include <unistd.h>

ssize_t write(int fd, const void* buffer, uint32_t count) {
	int result;

	asm volatile("int $0x80" : "=a"(result) : "a"(1), "b"(fd), "c"(buffer), "d"(count) : "memory");

	return result;
}
