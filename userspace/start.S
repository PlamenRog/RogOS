.global _start
.type _start, @function

.extern main
.extern exit

_start:
	xorl %ebp, %ebp

	call main

	pushl %eax
	call exit

1:
	jmp 1b
