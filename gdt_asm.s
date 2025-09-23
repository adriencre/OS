; gdt_asm.s
global gdt_flush
global gdt_ptr   ; <-- CETTE LIGNE EST CRUCIALE

section .data
gdt_start:
    ; Descripteur nul (obligatoire)
    dd 0x0
    dd 0x0

    ; Descripteur de segment de code noyau (Ring 0)
    dw 0xFFFF      ; Limite (bas)
    dw 0x0         ; Base (bas)
    db 0x0         ; Base (milieu)
    db 0x9A        ; Accès : Présent, Ring 0, Code, Exécutable/Lisible
    db 0xCF        ; Granularité : 4KB, mode 32-bit
    db 0x0         ; Base (haut)

    ; Descripteur de segment de données noyau (Ring 0)
    dw 0xFFFF      ; Limite (bas)
    dw 0x0         ; Base (bas)
    db 0x0         ; Base (milieu)
    db 0x92        ; Accès : Présent, Ring 0, Données, Lisible/Écrivable
    db 0xCF        ; Granularité : 4KB, mode 32-bit
    db 0x0         ; Base (haut)
gdt_end:

gdt_ptr:         ; <-- AINSI QUE CE LABEL EXACT
    dw gdt_end - gdt_start - 1 ; Taille de la GDT
    dd gdt_start               ; Adresse de la GDT

section .text
gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush
.flush:
    ret