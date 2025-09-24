#include "memory.h"

// Variables globales de l'allocateur
uint8_t heap[HEAP_SIZE] __attribute__((aligned(16)));
memory_block_t* heap_start = NULL;
int memory_initialized = 0;

// Fonctions externes nécessaires
void terminal_writestring(const char* data);
char* simple_itoa(int num, char* buffer, int base);

// Initialise l'allocateur de mémoire
void memory_init(void) {
    if (memory_initialized) {
        return;
    }
    
    // Initialise le premier bloc qui couvre tout le heap
    heap_start = (memory_block_t*)heap;
    heap_start->size = HEAP_SIZE - sizeof(memory_block_t);
    heap_start->free = 1;
    heap_start->next = NULL;
    
    SET_MAGIC(heap_start);
    memory_initialized = 1;
}

// Trouve un bloc libre de la taille demandée
memory_block_t* find_free_block(size_t size) {
    memory_block_t* current = heap_start;
    
    while (current) {
        if (current->free && GET_SIZE(current) >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Divise un bloc en deux si nécessaire
void split_block(memory_block_t* block, size_t size) {
    size_t block_size = GET_SIZE(block);
    
    if (block_size >= size + sizeof(memory_block_t) + BLOCK_SIZE) {
        memory_block_t* new_block = (memory_block_t*)((uint8_t*)block + sizeof(memory_block_t) + size);
        new_block->size = block_size - size - sizeof(memory_block_t);
        new_block->free = 1;
        new_block->next = block->next;
        SET_MAGIC(new_block);
        
        block->size = size;
        SET_MAGIC(block);
        block->next = new_block;
    }
}

// Alloue un bloc de mémoire
void* kmalloc(size_t size) {
    if (!memory_initialized) {
        memory_init();
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // Aligne la taille sur 16 bytes
    size = ALIGN_UP(size, 16);
    
    memory_block_t* block = find_free_block(size);
    if (!block) {
        return NULL;  // Pas assez de mémoire
    }
    
    // Divise le bloc si nécessaire
    split_block(block, size);
    
    // Marque le bloc comme utilisé
    block->free = 0;
    
    // Retourne un pointeur vers les données (après l'en-tête)
    return (void*)((uint8_t*)block + sizeof(memory_block_t));
}

// Fusionne les blocs libres adjacents
void merge_free_blocks(void) {
    memory_block_t* current = heap_start;
    
    while (current && current->next) {
        if (current->free && current->next->free) {
            // Vérifie si les blocs sont adjacents
            uint8_t* end_current = (uint8_t*)current + sizeof(memory_block_t) + GET_SIZE(current);
            if (end_current == (uint8_t*)current->next) {
                // Fusionne les blocs
                current->size = GET_SIZE(current) + sizeof(memory_block_t) + GET_SIZE(current->next);
                SET_MAGIC(current);
                current->next = current->next->next;
                continue;
            }
        }
        current = current->next;
    }
}

// Libère un bloc de mémoire
void kfree(void* ptr) {
    if (!ptr || !memory_initialized) {
        return;
    }
    
    // Récupère l'en-tête du bloc
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    // Vérifie le magic number
    if (!CHECK_MAGIC(block)) {
        return;  // Bloc corrompu ou invalide
    }
    
    // Marque le bloc comme libre
    block->free = 1;
    
    // Fusionne avec les blocs libres adjacents
    merge_free_blocks();
}

// Alloue et initialise à zéro
void* kcalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = kmalloc(total_size);
    
    if (ptr) {
        // Initialise à zéro
        uint8_t* bytes = (uint8_t*)ptr;
        for (size_t i = 0; i < total_size; i++) {
            bytes[i] = 0;
        }
    }
    
    return ptr;
}

// Redimensionne un bloc de mémoire
void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    // Récupère l'en-tête du bloc actuel
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    if (!CHECK_MAGIC(block)) {
        return NULL;  // Bloc invalide
    }
    
    size_t old_size = GET_SIZE(block);
    
    // Si la nouvelle taille est plus petite ou égale, on garde le même bloc
    if (new_size <= old_size) {
        return ptr;
    }
    
    // Alloue un nouveau bloc
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    // Copie les données
    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)new_ptr;
    for (size_t i = 0; i < old_size; i++) {
        dst[i] = src[i];
    }
    
    // Libère l'ancien bloc
    kfree(ptr);
    
    return new_ptr;
}

// Calcule la mémoire libre
size_t get_free_memory(void) {
    if (!memory_initialized) {
        return 0;
    }
    
    size_t free_memory = 0;
    memory_block_t* current = heap_start;
    
    while (current) {
        if (current->free) {
            free_memory += GET_SIZE(current);
        }
        current = current->next;
    }
    
    return free_memory;
}

// Calcule la mémoire utilisée
size_t get_used_memory(void) {
    if (!memory_initialized) {
        return 0;
    }
    
    size_t used_memory = 0;
    memory_block_t* current = heap_start;
    
    while (current) {
        if (!current->free) {
            used_memory += GET_SIZE(current);
        }
        // Ajoute la taille de l'en-tête
        used_memory += sizeof(memory_block_t);
        current = current->next;
    }
    
    return used_memory;
}

// Défragmente la mémoire
void memory_defragment(void) {
    merge_free_blocks();
}

// Affiche les statistiques mémoire
void print_memory_stats(void) {
    if (!memory_initialized) {
        terminal_writestring("Memoire non initialisee.\n");
        return;
    }
    
    char buffer[32];
    size_t free_mem = get_free_memory();
    size_t used_mem = get_used_memory();
    size_t total_mem = HEAP_SIZE;
    
    terminal_writestring("=== STATISTIQUES MEMOIRE ===\n");
    
    terminal_writestring("Total:     ");
    simple_itoa(total_mem, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("Utilisee:  ");
    simple_itoa(used_mem, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    terminal_writestring("Libre:     ");
    simple_itoa(free_mem, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring(" bytes\n");
    
    // Calcule le pourcentage d'utilisation
    int usage_percent = (used_mem * 100) / total_mem;
    terminal_writestring("Utilisation: ");
    simple_itoa(usage_percent, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring("%\n");
    
    // Compte le nombre de blocs
    int total_blocks = 0;
    int free_blocks = 0;
    memory_block_t* current = heap_start;
    
    while (current) {
        total_blocks++;
        if (current->free) {
            free_blocks++;
        }
        current = current->next;
    }
    
    terminal_writestring("Blocs totaux: ");
    simple_itoa(total_blocks, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("Blocs libres: ");
    simple_itoa(free_blocks, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring("\n");
    
    terminal_writestring("=============================\n");
}