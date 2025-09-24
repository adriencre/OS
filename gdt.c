// gdt.c
#include "common.h"

// Déclare gdt_ptr comme un label externe (le début d'un tableau non dimensionné).
// C'est la manière correcte de faire référence à un symbole d'assembleur.
extern char gdt_ptr[];

// Fonction assembleur pour charger la GDT (définie dans gdt_asm.s)
extern void gdt_flush(uint32_t);

// Fonction C pour installer la GDT
void gdt_install() {
    // Le nom d'un tableau en C est déjà son adresse de départ.
    gdt_flush((uint32_t)gdt_ptr);
}