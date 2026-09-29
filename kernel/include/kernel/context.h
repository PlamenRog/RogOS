#ifndef KERNEL_CONTEXT_H
#define KERNEL_CONTEXT_H

#include <kernel/process.h>

void context_switch(struct cpu_context* old, struct cpu_context* next);

#endif
