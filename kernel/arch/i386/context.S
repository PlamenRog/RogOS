.global context_switch

context_switch:
	pushfl
	pushl %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl 24(%esp), %eax
	movl 28(%esp), %edx

	# saves current registers
	movl %edi, 0(%eax)
	movl %esi, 4(%eax)
	movl %ebp, 8(%eax)

	leal 24(%esp), %ecx
	movl %ecx, 12(%eax)
	
	movl %ebx, 16(%eax)

	# return address of context_switch
	movl 20(%esp), %ecx
	movl %ecx, 20(%eax)

	# save eflags
	movl 16(%esp), %ecx
	movl %ecx, 24(%eax)

	# restores next context
	movl 0(%edx), %edi
	movl 4(%edx), %esi
	movl 8(%edx), %ebp
	movl 12(%edx), %esp
	movl 16(%edx), %ebx

	# restores eflags
	pushl 24(%edx)
	popfl

	# jump to saved eip
	jmp *20(%edx)
