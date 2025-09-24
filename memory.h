#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

// Constantes pour la gestion mémoire
#define HEAP_SIZE (1024 * 1024)  // 1MB de heap
#define BLOCK_SIZE 16            // Taille minimum d'un bloc
#define MAX_BLOCKS 1024          // Nombre maximum de blocs

// Structure pour un bloc de mémoire
typedef struct memory_block {
    size_t size;                 // Taille du bloc
    int free;                    // 1 si libre, 0 si utilisé
    struct memory_block* next;   // Pointeur vers le bloc suivant
} memory_block_t;

// Variables globales de l'allocateur
extern uint8_t heap[HEAP_SIZE];
extern memory_block_t* heap_start;
extern int memory_initialized;

// Fonctions de l'allocateur
void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t new_size);

// Fonctions utilitaires
size_t get_free_memory(void);
size_t get_used_memory(void);
void memory_defragment(void);
void print_memory_stats(void);

// Macros pour faciliter l'utilisation
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

// Protection contre la corruption mémoire
#define MAGIC_NUMBER 0xDEADBEEF
#define CHECK_MAGIC(block) ((block)->size & 0x80000000)
#define SET_MAGIC(block) ((block)->size |= 0x80000000)
#define CLEAR_MAGIC(block) ((block)->size &= 0x7FFFFFFF)
#define GET_SIZE(block) ((block)->size & 0x7FFFFFFF)

#endif // MEMORY_H