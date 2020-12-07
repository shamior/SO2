.global msg_syscall
msg_syscall:
	li a7, 1	# número da syscall
	ecall
	ret