.global enter_user_mode
.type enter_user_mode, @function

enter_user_mode:
	# 4(%esp) = user entry point
	# 8(%esp) = user stack

	movl 4(%esp), %eax
	movl 8(%esp), %ebx

	cli

	# ring 3 data selector
	movw $0x23, %cx

	movw %cx, %ds
	movw %cx, %es
	movw %cx, %fs
	movw %cx, %gs

	# privilege-changing iret frame:
	# SS
	# ESP
	# EFLAGS
	# CS
	# EIP

	pushl $0x23
	pushl %ebx

	# IF=1 and reserved bit 1 set
	pushl $0x202

	pushl $0x1B
	pushl %eax

	iret
