#include "kistory.h"
#include "timer.h"
#include "keyboard.h"

extern void terminal_writestring(const char* data);
extern void terminal_putchar(char c);
extern void clear_screen();
extern uint32_t tick;
extern int getkey();
extern int strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);
extern size_t strlen(const char* s);
extern char* strstr(const char* haystack, const char* needle);
extern char* simple_itoa(int num, char* buffer, int base);

history_entry_t history_commands[MAX_HISTORY_SIZE];
int history_count = 0;
int history_index = 0;
int history_navigation_index = 0;

void kistory_init() {
    history_count = 0;
    history_index = 0;
    history_navigation_index = 0;
    kistory_add_command("help");
    kistory_add_command("clear");
    kistory_add_command("about");
}

void kistory_add_command(const char* command) {
    if (!command || strlen(command) == 0) return;
    
    if (history_count > 0 && strcmp(history_commands[history_count - 1].command, command) == 0) {
        return;
    }
    
    if (history_count >= MAX_HISTORY_SIZE) {
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
            history_commands[i] = history_commands[i + 1];
        }
        history_count = MAX_HISTORY_SIZE - 1;
    }
    
    strcpy(history_commands[history_count].command, command);
    history_commands[history_count].timestamp = tick;
    history_commands[history_count].is_favorite = 0;
    history_count++;
    
    history_navigation_index = history_count;
}

const char* kistory_get_previous() {
    if (history_count == 0) return NULL;
    
    if (history_navigation_index > 0) {
        history_navigation_index--;
        return history_commands[history_navigation_index].command;
    }
    
    return NULL;
}

const char* kistory_get_next() {
    if (history_count == 0) return NULL;
    
    if (history_navigation_index < history_count) {
        history_navigation_index++;
        if (history_navigation_index >= history_count) {
            return "";
        }
        return history_commands[history_navigation_index].command;
    }
    
    return NULL;
}

void kistory_reset_navigation() {
    history_navigation_index = history_count;
}

void kistory_display_all() {
    clear_screen();
    terminal_writestring("=== HISTORIQUE DES COMMANDES (KISTORY) ===\n\n");
    
    if (history_count == 0) {
        terminal_writestring("Aucune commande dans l'historique.\n");
        return;
    }
    
    for (int i = 0; i < history_count; i++) {
        char num_str[4];
        simple_itoa(i + 1, num_str, 10);
        terminal_writestring(num_str);
        terminal_writestring(". ");
        
        if (history_commands[i].is_favorite) {
            terminal_writestring("★ ");
        } else {
            terminal_writestring("  ");
        }
        
        terminal_writestring(history_commands[i].command);
        terminal_writestring("\n");
    }
    
    terminal_writestring("\nAppuyez sur une touche pour continuer...");
    getkey();
}

void kistory_clear() {
    history_count = 0;
    history_navigation_index = 0;
    terminal_writestring("Historique efface.\n");
}

void kistory_search(const char* query) {
    if (!query || strlen(query) == 0) {
        terminal_writestring("Requete de recherche vide.\n");
        return;
    }
    
    clear_screen();
    terminal_writestring("=== RECHERCHE DANS L'HISTORIQUE ===\n");
    terminal_writestring("Recherche: ");
    terminal_writestring(query);
    terminal_writestring("\n\n");
    
    int found = 0;
    for (int i = 0; i < history_count; i++) {
        if (strstr(history_commands[i].command, query)) {
            found++;
            
            char num_str[4];
            simple_itoa(i + 1, num_str, 10);
            terminal_writestring(num_str);
            terminal_writestring(". ");
            
            if (history_commands[i].is_favorite) {
                terminal_writestring("★ ");
            } else {
                terminal_writestring("  ");
            }
            
            terminal_writestring(history_commands[i].command);
            terminal_writestring("\n");
        }
    }
    
    if (found == 0) {
        terminal_writestring("Aucun resultat trouve.\n");
    }
    
    terminal_writestring("\nAppuyez sur une touche pour continuer...");
    getkey();
}

void kistory_toggle_favorite(int index) {
    if (index >= 0 && index < history_count) {
        history_commands[index].is_favorite = !history_commands[index].is_favorite;
        
        if (history_commands[index].is_favorite) {
            terminal_writestring("Commande ajoutee aux favoris.\n");
        } else {
            terminal_writestring("Commande retiree des favoris.\n");
        }
    } else {
        terminal_writestring("Index invalide.\n");
    }
}

void kistory_display_favorites() {
    clear_screen();
    terminal_writestring("=== COMMANDES FAVORITES ===\n\n");
    
    int favorites_count = 0;
    for (int i = 0; i < history_count; i++) {
        if (history_commands[i].is_favorite) {
            favorites_count++;
            
            char num_str[4];
            simple_itoa(favorites_count, num_str, 10);
            terminal_writestring(num_str);
            terminal_writestring(". ");
            terminal_writestring(history_commands[i].command);
            terminal_writestring("\n");
        }
    }
    
    if (favorites_count == 0) {
        terminal_writestring("Aucune commande favorite.\n");
    }
    
    terminal_writestring("\nAppuyez sur une touche pour continuer...");
    getkey();
}

void kistory_system() {
    while (1) {
        clear_screen();
        terminal_writestring("=== KISTORY - SYSTEME D'HISTORIQUE ===\n\n");
        terminal_writestring("1. Afficher l'historique complet\n");
        terminal_writestring("2. Rechercher dans l'historique\n");
        terminal_writestring("3. Afficher les favoris\n");
        terminal_writestring("4. Effacer l'historique\n");
        terminal_writestring("5. Retour au shell\n");
        terminal_writestring("\nChoix: ");
        
        int choice = getkey() - '0';
        terminal_putchar('\n');
        
        switch (choice) {
            case 1:
                kistory_display_all();
                break;
            case 2: {
                char query[64];
                terminal_writestring("Entrez votre recherche: ");
                
                size_t len = 0;
                while (len < 63) {
                    int key = getkey();
                    if (key == '\n') break;
                    if (key == '\b' && len > 0) {
                        len--;
                        terminal_putchar('\b');
                        terminal_putchar(' ');
                        terminal_putchar('\b');
                    } else if (key > 0 && key < 256) {
                        query[len] = (char)key;
                        len++;
                        terminal_putchar(key);
                    }
                }
                query[len] = '\0';
                terminal_putchar('\n');
                
                kistory_search(query);
                break;
            }
            case 3:
                kistory_display_favorites();
                break;
            case 4:
                kistory_clear();
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
                break;
            case 5:
                return;
            default:
                terminal_writestring("Choix invalide.\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
        }
    }
}
