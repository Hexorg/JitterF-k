section .text
	global _start

_start:
mov rax, 0102030405060708h
mov eax, 01020304h
mov ax, 0102h

mov rbx, 0102030405060708h
mov ebx, 01020304h
mov bx, 0102h

mov rcx, 0102030405060708h

mov ecx, 01020304h
mov cx, 0102h

mov rdx, 0102030405060708h
mov edx, 01020304h
mov dx, 0102h

inc rax
inc rbx
inc rcx
inc rdx

inc eax
inc ebx
inc ecx
inc edx

dec rax
dec rbx
dec rcx
dec rdx

dec eax
dec ebx
dec ecx
dec edx

inc byte [rbx+rcx]
inc byte [ebx+ecx]

dec byte [rbx+rcx]
dec byte [ebx+ecx]

mov al, [rbx+rcx]
mov eax, [ebx+ecx]

mov [rbx+rcx], rax
mov [ebx+ecx], al

xor rax, rax

test rcx, rcx
test ecx, ecx
test rax, rax
test eax, eax
test al, al

je 0102030405060708h
je 01020304h

push rax

pop rax

push rbx

pop rbx

push rcx

pop rcx

jmp 0102030405060708h
jmp 01020304h

call 0102030405060708h
call 01020304h

ret
leave
