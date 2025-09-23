section .multiboot
align 4
    dd 0x1BADB002   ; magic
    dd 0x00         ; flags
    dd -0x1BADB002  ; checksum

section .text
global _start
_start:
    ; Le code C s'attend à une pile (stack) valide
    mov esp, stack_space
    
    extern kernel_main
    call kernel_main

    ; Boucle infinie si kernel_main retourne
    cli
.hang:
    hlt
    jmp .hang

section .bss
resb 8192 ; 8KB pour la pile
stack_space: