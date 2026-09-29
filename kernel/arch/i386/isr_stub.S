.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31

.extern isr_handler

# exceptions without an error code
.macro ISR_NOERR num
isr\num:
	cli
	pushl $0 # dummy error code
	pushl $\num # interrupt number
	jmp isr_common
.endm

# exceptions with an error code
.macro ISR_ERR num
isr\num:
	cli
	pushl $\num # cpu already pushed the error code
	jmp isr_common
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7

ISR_ERR 8

ISR_NOERR 9

ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14

ISR_NOERR 15
ISR_NOERR 16

ISR_ERR 17

ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20

ISR_ERR 21

ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR 29
ISR_ERR 30
ISR_NOERR 31

isr_common:

	pusha

	push %ds
	push %es
	push %fs
	push %gs

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	push %esp
	call isr_handler
	add $4, %esp

	pop %gs
	pop %fs
	pop %es
	pop %ds

	popa

	add $8, %esp

	iret

	
.align 4
.global isr_stub_table

isr_stub_table:
	.long isr0
	.long isr1
	.long isr2
	.long isr3
	.long isr4
	.long isr5
	.long isr6
	.long isr7
	.long isr8
	.long isr9
	.long isr10
	.long isr11
	.long isr12
	.long isr13
	.long isr14
	.long isr15
	.long isr16
	.long isr17
	.long isr18
	.long isr19
	.long isr20
	.long isr21
	.long isr22
	.long isr23
	.long isr24
	.long isr25
	.long isr26
	.long isr27
	.long isr28
	.long isr29
	.long isr30
	.long isr31

.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15

.extern irq_handler


.macro IRQ num

irq\num:
	cli
	pushl $0
	pushl $(32+\num)
	jmp irq_common

.endm


IRQ 0
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQ 8
IRQ 9
IRQ 10
IRQ 11
IRQ 12
IRQ 13
IRQ 14
IRQ 15


irq_common:

	pusha

	push %ds
	push %es
	push %fs
	push %gs
	
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	push %esp
	call irq_handler
	add $4, %esp

	pop %gs
	pop %fs
	pop %es
	pop %ds

	popa

	add $8, %esp

	iret
