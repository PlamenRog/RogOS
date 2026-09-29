HOST ?= i686-elf
CC := $(HOST)-gcc

SYSROOT ?= ../sysroot

CFLAGS ?= -O2 -g

PROGRAM_SOURCES := $(wildcard programs/*.c)
PROGRAM_NAMES := $(notdir $(PROGRAM_SOURCES:.c=))
PROGRAM_TARGETS := $(addprefix $(SYSROOT)/usr/bin/,$(PROGRAM_NAMES))


.PHONY: all clean

all: $(PROGRAM_TARGETS)


$(SYSROOT)/usr/bin/%: programs/%.c start.S linker.ld
	mkdir -p $(SYSROOT)/usr/bin

	$(CC) \
		--sysroot=$(SYSROOT) \
		-isystem $(SYSROOT)/usr/include \
		$(CFLAGS) \
		-ffreestanding \
		-fno-pie \
		-nostdlib \
		-nostartfiles \
		-Wl,-T,linker.ld \
		-L$(SYSROOT)/usr/lib \
		start.S \
		$< \
		-lc \
		-lgcc \
		-o $@


clean:
	rm -f $(PROGRAM_TARGETS)
