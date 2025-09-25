#include "timer.h"
#include "idt.h"
#include "common.h"
#include "messaging.h"

uint32_t tick = 0;

// Cette fonction est appelée par le wrapper assembleur de l'IRQ0
void timer_handler(void* regs) {
    (void)regs; // Pour éviter l'avertissement "unused parameter"
    tick++;
    
    // Vérifier les messages automatiques à chaque tick (affichage en temps réel)
    messaging_check_auto_messages();
    
    // Envoyer un signal de fin d'interruption (End of Interrupt) au PIC
    outb(0x20, 0x20);
}

// Initialise le PIT
void timer_install() {
    uint32_t frequency = 100; // Fréquence en Hz
    uint32_t divisor = 1193180 / frequency;

    // Envoie l'octet de commande (mode 2, générateur de fréquence)
    outb(0x43, 0x36);

    // Envoie le diviseur de fréquence (octet bas, puis octet haut)
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);
}