BITS    64

section .text
	global _start

_start:
	mov rax, 0x4
	mov rbx, 0x1 
	mov rcx, woody_str
	mov rdx, 16
	int	0x80
	xor rax, rax
	xor rdi, rdi
	xor rdx, rdx
	xor rsi, rsi
	jmp 0xffffffff

align 8
	woody_str db '....WOODY.....', 0xA, 0x0