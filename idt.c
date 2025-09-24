// idt.c (mis à jour)
#include "idt.h"
#include "common.h"

// Nos tables et pointeurs IDT sont maintenant des variables globales
// mais leurs types sont définis dans idt.h
struct idt_entry idt[256];
struct idt_ptr   idtp;

// Fonctions assembleur externes (wrappers)
extern void idt_load(uint32_t);
extern void isr0();
extern void irq0();

// Remappage du PIC (Programmable Interrupt Controller)
void pic_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master PIC: IRQs 0-7  -> Ints 32-39
    outb(0xA1, 0x28); // Slave PIC:  IRQs 8-15 -> Ints 40-47
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

// Fonction pour définir une porte d'interruption
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags | 0x60;
}

// Fonction principale d'installation de l'IDT
void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    pic_remap();

    // Masque toutes les IRQs sauf celle du timer (IRQ 0)
    outb(0x21, 0xFE); 
    outb(0xA1, 0xFF);

    // Ajoute les gestionnaires d'interruptions
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);  // Exception 0
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E); // IRQ 0 (Timer)

    // Charge la table IDT
    idt_load((uint32_t)&idtp);
}

// Gestionnaire d'exceptions C
void fault_handler(void *regs) {
    (void)regs;
    volatile uint16_t* vga = (uint16_t*)0xB8000;
    const char* msg = "CPU EXCEPTION!";
    for (int i = 0; msg[i] != '\0'; ++i) {
        vga[i] = (0x4F << 8) | msg[i]; // Texte blanc sur fond rouge
    }
    for (;;);
}