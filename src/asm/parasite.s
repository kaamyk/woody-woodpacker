BITS    64

section .text
	global _start

_start:
	mov rdi, 0x1
	mov rsi, woody_str
	mov rdx, 0x10 
	syscall
	xor rax, rax
	xor rdi, rdi
	xor rdx, rdx
	xor rsi, rsi
	jmp 0xffffffff

align 8
	woody_str db '....WOODY.....', 0xA, 0x0