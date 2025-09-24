extern fault_handler
extern timer_handler

global idt_load
global isr0
global irq0

section .text

idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret

; ISR 0 : Exception
isr0:
    cli
    push 0
    push 0
    pushad
    mov eax, esp
    push eax
    call fault_handler
    pop eax
    popad
    add esp, 8
    sti
    iret

; IRQ 0 : Timer
irq0:
    cli
    pushad
    mov eax, esp
    push eax
    call timer_handler
    pop eax
    popad
    sti
    iret