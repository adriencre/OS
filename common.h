// common.h
#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h> // <-- CETTE LIGNE CORRIGE LES ERREURS

// Fonctions d'E/S sur les ports
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

#endif