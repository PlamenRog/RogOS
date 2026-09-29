#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <kernel/isr.h>
#include <stdint.h>

#define SYS_EXIT 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_READ 4

void syscall_initialize(void);

void syscall_handler(struct registers* regs);

#endif
