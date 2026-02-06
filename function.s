.intel_syntax noprefix
.globl plus, main

plus:
        add rsi, rdi
        mov rax, rsi
        ret

main:
        mov rdi, 0
        mov rsi, 0
        call plus
        ret