#include <kernel/power.h>

#include <stdint.h>

static inline void outw(uint16_t port, uint16_t value) {
	__asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

void power_shutdown(void) {
	outw(0x604, 0x2000);

	for (;;) {
		__asm__ volatile("cli; hlt");
	}
}
