#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t ssize_t;

ssize_t write(int fd, const void* buffer, uint32_t count);

ssize_t read(int fd, void* buffer, uint32_t count);

int close(int fd);

#ifdef __cplusplus
}
#endif

#endif
