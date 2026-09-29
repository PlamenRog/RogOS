.global syscall_stub
.extern syscall_handler

syscall_stub:
	cli

	# fake interrupt frame
	pushl $0
	pushl $0x80

	# save general purpose registers
	pusha

	# save segment registers as 32bit values
	xorl %eax, %eax
	movw %ds, %ax
	pushl %eax

	xorl %eax, %eax
	movw %es, %ax
	pushl %eax

	xorl %eax, %eax
	movw %fs, %ax
	pushl %eax

	xorl %eax, %eax
	movw %gs, %ax
	pushl %eax

	# switch to kernel data segments.
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	# syscall_handler(struct registers*)
	pushl %esp
	call syscall_handler
	addl $4, %esp

	# restore segment registers.
	popl %eax
	movw %ax, %gs

	popl %eax
	movw %ax, %fs

	popl %eax
	movw %ax, %es

	popl %eax
	movw %ax, %ds

	popa

	# remove fake err_code + int_no.
	addl $8, %esp

	iret
