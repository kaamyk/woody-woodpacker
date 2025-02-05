BITS    32

section .data
woody_str db '....WOODY.....', 0xA, 0xD , 0x0

section .text
	global _start

_start:
	pusha
	mov eax, 0x4
	mov ebx, 0x1 
	mov ecx, woody_str
	mov edx, 16
	int	0x80
	popa
	jmp 0xffffffff