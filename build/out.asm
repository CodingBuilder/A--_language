section .text
   global main
main:
   mov rax, 100
   push rax
   mov rax, 60
   pop rdi
   syscall
