#include "filemanager.h"
#include <stddef.h>

// --- Fonctions Externes du Kernel ---
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void clear_screen(void);
int getkey();
extern void readline(char* buffer, size_t max_len);
char* simple_itoa(int num, char* buffer, int base);
int simple_atoi(const char* str);
int strcmp(const char* s1, const char* s2);
int strlen(const char* str);

// --- Fonctions graphiques du Kernel ---
void terminal_putentryat(char c, uint8_t color, int x, int y);

// --- Couleurs et Affichage Amélioré ---
#define COLOR_BLACK          0
#define COLOR_BLUE           1
#define COLOR_GREEN          2
#define COLOR_CYAN           3
#define COLOR_RED            4
#define COLOR_YELLOW         14
#define COLOR_WHITE          15
#define VGA_COLOR(fg, bg)    (fg | bg << 4)

// --- Fonctions d'Aide à l'Affichage ---
void fm_print_at_color(const char* str, uint8_t color, int x, int y) {
    int i = 0;
    while (str[i] != '\0') {
        terminal_putentryat(str[i], color, x + i, y);
        i++;
    }
}

void fm_draw_box(const char* title, int x, int y, int width, int height) {
    uint8_t border_color = VGA_COLOR(COLOR_BLUE, COLOR_BLACK);
    uint8_t title_color = VGA_COLOR(COLOR_YELLOW, COLOR_BLACK);

    terminal_putentryat('+', border_color, x, y);
    terminal_putentryat('+', border_color, x + width - 1, y);
    terminal_putentryat('+', border_color, x, y + height - 1);
    terminal_putentryat('+', border_color, x + width - 1, y + height - 1);

    for (int i = 1; i < width - 1; i++) {
        terminal_putentryat('-', border_color, x + i, y);
        terminal_putentryat('-', border_color, x + i, y + height - 1);
    }
    for (int i = 1; i < height - 1; i++) {
        terminal_putentryat('|', border_color, x, y + i);
        terminal_putentryat('|', border_color, x + width - 1, y + i);
    }
    fm_print_at_color(title, title_color, x + 2, y);
}

void fm_clear_area(int x, int y, int width, int height) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            terminal_putentryat(' ', VGA_COLOR(COLOR_BLACK, COLOR_BLACK), x + i, y + j);
        }
    }
}

// --- Système de Fichiers Réel ---
#define MAX_FILES 50
#define MAX_FILENAME 64

typedef struct {
    char name[MAX_FILENAME];
    int is_directory;
    int size;
    int exists;
} real_file_t;

// Liste des fichiers réels du système
static real_file_t real_files[] = {
    // Fichiers sources C
    {"kernel.c", 0, 0, 1},
    {"idt.c", 0, 0, 1},
    {"gdt.c", 0, 0, 1},
    {"io.c", 0, 0, 1},
    {"timer.c", 0, 0, 1},
    {"keyboard.c", 0, 0, 1},
    {"game.c", 0, 0, 1},
    {"health.c", 0, 0, 1},
    {"blackjack.c", 0, 0, 1},
    {"memory.c", 0, 0, 1},
    {"editor.c", 0, 0, 1},
    {"filemanager.c", 0, 0, 1},
    
    // Fichiers headers
    {"common.h", 0, 0, 1},
    {"idt.h", 0, 0, 1},
    {"gdt.h", 0, 0, 1},
    {"timer.h", 0, 0, 1},
    {"keyboard.h", 0, 0, 1},
    {"game.h", 0, 0, 1},
    {"health.h", 0, 0, 1},
    {"blackjack.h", 0, 0, 1},
    {"memory.h", 0, 0, 1},
    {"editor.h", 0, 0, 1},
    {"filemanager.h", 0, 0, 1},
    
    // Fichiers assembly
    {"boot.s", 0, 0, 1},
    {"gdt_asm.s", 0, 0, 1},
    {"interrupts.s", 0, 0, 1},
    
    // Fichiers objets
    {"kernel.o", 0, 0, 1},
    {"idt.o", 0, 0, 1},
    {"gdt.o", 0, 0, 1},
    {"io.o", 0, 0, 1},
    {"timer.o", 0, 0, 1},
    {"keyboard.o", 0, 0, 1},
    {"game.o", 0, 0, 1},
    {"health.o", 0, 0, 1},
    {"blackjack.o", 0, 0, 1},
    {"memory.o", 0, 0, 1},
    {"editor.o", 0, 0, 1},
    {"filemanager.o", 0, 0, 1},
    {"boot.o", 0, 0, 1},
    {"gdt_asm.o", 0, 0, 1},
    {"interrupts.o", 0, 0, 1},
    
    // Fichiers binaires
    {"kernel.bin", 0, 0, 1},
    {"myos.iso", 0, 0, 1},
    
    // Fichiers de configuration
    {"Makefile", 0, 0, 1},
    {"linker.ld", 0, 0, 1},
    {"README_background.md", 0, 0, 1},
    
    // Répertoires
    {"isofiles/", 1, 0, 1},
    {"boot/", 1, 0, 1},
    {"grub/", 1, 0, 1}
};

static const int num_real_files = sizeof(real_files) / sizeof(real_file_t);

// Fonction pour obtenir le type de fichier par extension
const char* get_file_type(const char* filename) {
    int len = strlen(filename);
    if (len < 2) return "???";
    
    // Chercher l'extension
    for (int i = len - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            const char* ext = &filename[i + 1];
            if (strcmp(ext, "c") == 0) return "C";
            if (strcmp(ext, "h") == 0) return "H";
            if (strcmp(ext, "s") == 0) return "ASM";
            if (strcmp(ext, "o") == 0) return "OBJ";
            if (strcmp(ext, "bin") == 0) return "BIN";
            if (strcmp(ext, "iso") == 0) return "ISO";
            if (strcmp(ext, "ld") == 0) return "LD";
            if (strcmp(ext, "md") == 0) return "MD";
            return "???";
        }
    }
    
    // Vérifier si c'est un répertoire
    if (filename[len - 1] == '/') return "DIR";
    
    // Fichiers sans extension
    if (strcmp(filename, "Makefile") == 0) return "MAK";
    
    return "???";
}

// Fonction pour obtenir la taille simulée d'un fichier
int get_file_size(const char* filename) {
    // Simulation de tailles basée sur le type de fichier
    int len = strlen(filename);
    
    if (filename[len - 1] == '/') return 0; // Répertoire
    
    for (int i = len - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            const char* ext = &filename[i + 1];
            if (strcmp(ext, "c") == 0) return 5000 + (len * 100); // Fichiers C
            if (strcmp(ext, "h") == 0) return 1000 + (len * 50);  // Headers
            if (strcmp(ext, "s") == 0) return 2000 + (len * 80);  // Assembly
            if (strcmp(ext, "o") == 0) return 3000 + (len * 60);  // Objets
            if (strcmp(ext, "bin") == 0) return 50000;            // Binaires
            if (strcmp(ext, "iso") == 0) return 1000000;          // ISO
            if (strcmp(ext, "ld") == 0) return 500;               // Linker
            if (strcmp(ext, "md") == 0) return 2000;              // Markdown
            break;
        }
    }
    
    if (strcmp(filename, "Makefile") == 0) return 1500;
    
    return 1000; // Taille par défaut
}

// --- Fonctions de l'Application ---
void list_all_files() {
    uint8_t header_color = VGA_COLOR(COLOR_YELLOW, COLOR_BLACK);
    uint8_t data_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);
    uint8_t dir_color = VGA_COLOR(COLOR_CYAN, COLOR_BLACK);
    uint8_t src_color = VGA_COLOR(COLOR_GREEN, COLOR_BLACK);

    fm_clear_area(0, 5, 80, 17);
    
    fm_print_at_color(" Nom du Fichier ", header_color, 4, 6);
    fm_print_at_color("| Type ", header_color, 25, 6);
    fm_print_at_color("| Taille ", header_color, 35, 6);
    fm_print_at_color("| Perms ", header_color, 50, 6);
    fm_print_at_color("------------------+--------+----------+-------", header_color, 4, 7);

    for (int i = 0; i < num_real_files; i++) {
        int y = 8 + i;
        if (y > 20) break; // Limiter l'affichage
        
        real_file_t* f = &real_files[i];
        uint8_t color = f->is_directory ? dir_color : 
                       (strcmp(get_file_type(f->name), "C") == 0 || 
                        strcmp(get_file_type(f->name), "H") == 0) ? src_color : data_color;
        
        fm_print_at_color(f->name, color, 5, y);
        
        // Type
        const char* type_str = get_file_type(f->name);
        fm_print_at_color(type_str, data_color, 27, y);
        
        // Taille
        if (f->is_directory) {
            fm_print_at_color("-", data_color, 37, y);
        } else {
            char size_buffer[12];
            int size = get_file_size(f->name);
            simple_itoa(size, size_buffer, 10);
            fm_print_at_color(size_buffer, data_color, 37, y);
        }
        
        // Permissions
        fm_print_at_color("rw-", data_color, 52, y);
    }

    fm_print_at_color("Appuyez sur une touche pour continuer...", VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 4, 22);
    getkey();
}

void show_file_info(int index) {
    if (index < 0 || index >= num_real_files) {
        fm_print_at_color("Erreur : Fichier introuvable.", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
        return;
    }
    
    real_file_t* f = &real_files[index];
    char size_buffer[12];

    fm_draw_box(" Informations Fichier ", 20, 6, 40, 9);
    uint8_t label_color = VGA_COLOR(COLOR_CYAN, COLOR_BLACK);
    uint8_t data_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);

    fm_print_at_color("Nom............: ", label_color, 22, 8); fm_print_at_color(f->name, data_color, 40, 8);
    
    const char* type_str = get_file_type(f->name);
    fm_print_at_color("Type............: ", label_color, 22, 9); fm_print_at_color(type_str, data_color, 40, 9);
    
    if (f->is_directory) {
        fm_print_at_color("Taille..........: ", label_color, 22, 10); fm_print_at_color("-", data_color, 40, 10);
    } else {
        int size = get_file_size(f->name);
        simple_itoa(size, size_buffer, 10);
        fm_print_at_color("Taille..........: ", label_color, 22, 10); fm_print_at_color(size_buffer, data_color, 40, 10);
    }
    
    fm_print_at_color("Permissions....: ", label_color, 22, 11); fm_print_at_color("rw-", data_color, 40, 11);
    fm_print_at_color("Existe..........: ", label_color, 22, 12); fm_print_at_color("Oui", data_color, 40, 12);
}

void read_file_content(int index) {
    if (index < 0 || index >= num_real_files) {
        fm_print_at_color("Erreur : Fichier introuvable.", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
        return;
    }
    
    real_file_t* f = &real_files[index];
    
    if (f->is_directory) {
        fm_print_at_color("Impossible de lire un repertoire.", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
        return;
    }
    
    fm_draw_box(" Contenu du Fichier ", 10, 6, 60, 15);
    fm_print_at_color(f->name, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 12, 7);
    
    // Afficher des informations sur le fichier
    fm_print_at_color("Type de fichier:", VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 12, 9);
    fm_print_at_color(get_file_type(f->name), VGA_COLOR(COLOR_WHITE, COLOR_BLACK), 30, 9);
    
    fm_print_at_color("Taille estimee:", VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 12, 10);
    char size_buffer[12];
    simple_itoa(get_file_size(f->name), size_buffer, 10);
    fm_print_at_color(size_buffer, VGA_COLOR(COLOR_WHITE, COLOR_BLACK), 30, 10);
    
    fm_print_at_color("Ce fichier fait partie du projet NOVA.", VGA_COLOR(COLOR_GREEN, COLOR_BLACK), 12, 12);
    fm_print_at_color("Il s'agit d'un vrai fichier du systeme.", VGA_COLOR(COLOR_GREEN, COLOR_BLACK), 12, 13);
    
    fm_print_at_color("Appuyez sur une touche pour continuer...", VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 12, 21);
    getkey();
}

void fm_display_help() {
    fm_draw_box(" Aide ", 20, 6, 40, 10);
    uint8_t cmd_color = VGA_COLOR(COLOR_YELLOW, COLOR_BLACK);
    uint8_t desc_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);
    fm_print_at_color("liste", cmd_color, 22, 8);
    fm_print_at_color("- Voir tous les fichiers", desc_color, 32, 8);
    fm_print_at_color("info <num>", cmd_color, 22, 9);
    fm_print_at_color("- Details d'un fichier", desc_color, 32, 9);
    fm_print_at_color("lire <num>", cmd_color, 22, 10);
    fm_print_at_color("- Lire un fichier", desc_color, 32, 10);
    fm_print_at_color("quitter", cmd_color, 22, 11);
    fm_print_at_color("- Fermer l'application", desc_color, 32, 11);
    fm_print_at_color("aide", cmd_color, 22, 12);
    fm_print_at_color("- Afficher cette aide", desc_color, 32, 12);
}

void parse_file_command(char* line, char* command, char* args) {
    int i = 0;
    while (line[i] != ' ' && line[i] != '\0') { command[i] = line[i]; i++; }
    command[i] = '\0';
    if (line[i] == ' ') {
        i++;
        int j = 0;
        while (line[i] != '\0') { args[j] = line[i]; j++; i++; }
        args[j] = '\0';
    } else {
        args[0] = '\0';
    }
}

// Point d'entrée principal
void run_file_manager(void) {
    clear_screen();
    uint8_t title_color = VGA_COLOR(COLOR_CYAN, COLOR_BLACK);
    const char* title = "=== Gestionnaire de Fichiers NOVA ===";
    fm_print_at_color(title, title_color, (80 - 35) / 2, 2);
    
    // Afficher le répertoire courant
    fm_print_at_color("Repertoire courant: / (racine du projet NOVA)", VGA_COLOR(COLOR_GREEN, COLOR_BLACK), 2, 4);
    
    fm_display_help();

    char input_buffer[128], command[64], args[64];

    while (1) {
        fm_clear_area(0, 22, 80, 2);
        fm_print_at_color("Fichiers> ", VGA_COLOR(COLOR_GREEN, COLOR_BLACK), 2, 23);
        readline(input_buffer, sizeof(input_buffer));

        parse_file_command(input_buffer, command, args);

        fm_clear_area(0, 5, 80, 17);

        if (strcmp(command, "liste") == 0) {
            list_all_files();
            fm_clear_area(0, 5, 80, 17);
            fm_display_help();
        } else if (strcmp(command, "info") == 0) {
            if (args[0] != '\0') {
                show_file_info(simple_atoi(args));
            } else {
                fm_print_at_color("Usage: info <numero>", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
            }
        } else if (strcmp(command, "lire") == 0) {
            if (args[0] != '\0') {
                read_file_content(simple_atoi(args));
            } else {
                fm_print_at_color("Usage: lire <numero>", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
            }
        } else if (strcmp(command, "quitter") == 0) {
            fm_print_at_color("Fermeture du gestionnaire...", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 2, 22);
            break;
        } else if (command[0] != '\0') {
            fm_display_help();
            if (strcmp(command, "aide") != 0) {
                 fm_print_at_color("Commande inconnue.", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
            }
        } else {
            fm_display_help();
        }
    }
    
    // Retour au shell
    clear_screen();
    terminal_writestring("Retour au shell principal.\n");
}