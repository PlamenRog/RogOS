# multiboot header constants
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# multiboot standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

# kernel entry point
.section .text
.global _start
.type _start, @function
_start:
	movl $stack_top, %esp

	pushl %ebx # multiboot info pointer
	pushl %eax # multiboot magic

	call _init

	call kernel_main

	add $8, %esp

	# hangs if kernel_main randomly returns
	cli
1:	hlt
	jmp 1b
.size _start, . - _start
