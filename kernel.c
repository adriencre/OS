#include "common.h"
#include "timer.h"
#include "idt.h"
#include "keyboard.h"
#include "game.h"
#include "pong.h"
#include "health.h"
#include "blackjack.h"
#include "memory.h"
#include "editor.h"
#include "filemanager.h"
#include "password.h"
#include "messaging.h"
#include "wiki.h"

// Déclarations de fonctions externes
void gdt_install();
extern uint32_t tick;

// --- Système d'historique des commandes amélioré ---
#define MAX_HISTORY_SIZE 20
#define MAX_COMMAND_LENGTH 256

static char history_commands[MAX_HISTORY_SIZE][MAX_COMMAND_LENGTH];
static int history_count = 0;
static int history_index = 0;

// Déclarations des fonctions de gestion de l'historique
void add_to_history(const char* command);
const char* get_history_command(int direction);
void reset_history_navigation();

// Déclarations des fonctions string
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcpy(char* dest, const char* src);
size_t strlen(const char* s);
char* strchr(const char* s, int c);

// --- Variables et fonctions VGA ---
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;
uint8_t terminal_color = 0x0F;
int cursor_x = 0;
int cursor_y = 0;

void move_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// ✅ FONCTION AJOUTÉE NÉCESSAIRE POUR BLACKJACK.C
// Affiche un caractère avec une couleur spécifique à une position (x, y)
void terminal_putentryat(char c, uint8_t color, int x, int y) {
    const int index = y * VGA_WIDTH + x;
    vga_buffer[index] = (uint16_t)c | (uint16_t)color << 8;
}

void terminal_putchar_at(char c, int x, int y) {
    // Protection contre les accès hors limites
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga_buffer[y * VGA_WIDTH + x] = ((uint16_t)terminal_color << 8) | c;
    }
}

void clear_screen() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            terminal_putchar_at(' ', x, y);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    move_cursor(cursor_x, cursor_y);
}

void scroll_screen() {
    // Faire défiler toutes les lignes vers le haut
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    // Vider la dernière ligne
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ((uint16_t)terminal_color << 8) | ' ';
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        terminal_putchar_at(c, cursor_x, cursor_y);
        cursor_x++;
    }
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    // Gérer le défilement si on dépasse l'écran
    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
        cursor_y = VGA_HEIGHT - 1;
    }
    move_cursor(cursor_x, cursor_y);
}

void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != 0; i++) {
        terminal_putchar(data[i]);
    }
}

// --- Lecture de ligne du Shell ---
void redraw_line(int start_x, char* buffer, size_t len) {
    for(size_t i = 0; i < len && (start_x + i) < VGA_WIDTH; i++) {
        terminal_putchar_at(buffer[i], start_x + i, cursor_y);
    }
    if ((start_x + len) < VGA_WIDTH) {
        terminal_putchar_at(' ', start_x + len, cursor_y);
    }
}

void readline(char* buffer, size_t max_len) {
    size_t len = 0, pos = 0;
    int start_x = cursor_x, start_y = cursor_y;
    
    // Réinitialiser la navigation dans l'historique
    reset_history_navigation();
    
    while (1) {
        int key = getkey();
        
        if (key == '\n') {
            terminal_putchar('\n');
            break;
        } else if (key == '\b') {
            if (pos > 0) {
                for (size_t i = pos - 1; i < len; i++) buffer[i] = buffer[i + 1];
                len--; pos--;
                redraw_line(start_x, buffer, len);
            }
        } else if (key == 257) { // KEY_UP - commande précédente
            const char* hist_cmd = get_history_command(-1);
            if (hist_cmd != NULL) {
                // Effacer la ligne actuelle
                for (size_t i = 0; i < len; i++) {
                    terminal_putchar_at(' ', start_x + i, start_y);
                }
                // Copier la commande de l'historique
                strcpy(buffer, hist_cmd);
                len = strlen(buffer);
                pos = len;
                redraw_line(start_x, buffer, len);
            }
        } else if (key == 258) { // KEY_DOWN - commande suivante
            const char* hist_cmd = get_history_command(1);
            if (hist_cmd != NULL) {
                // Effacer la ligne actuelle
                for (size_t i = 0; i < len; i++) {
                    terminal_putchar_at(' ', start_x + i, start_y);
                }
                // Copier la commande de l'historique (ou chaîne vide)
                strcpy(buffer, hist_cmd);
                len = strlen(buffer);
                pos = len;
                redraw_line(start_x, buffer, len);
            }
        } else if (key > 0 && key < 256 && len < max_len - 1) {
            for (size_t i = len; i > pos; i--) buffer[i] = buffer[i - 1];
            buffer[pos] = (char)key;
            len++; pos++;
            redraw_line(start_x, buffer, len);
        }
        
        cursor_x = start_x + pos;
        cursor_y = start_y;
        move_cursor(cursor_x, cursor_y);
    }
    buffer[len] = '\0';
}

// --- Fonctions utilitaires ---
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == c) return (char*)s;
        s++;
    }
    return (*s == c) ? (char*)s : NULL;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

void reverse_string(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

char* simple_itoa(int num, char* buffer, int base) {
    int i = 0;
    int is_negative = 0;
    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return buffer;
    }
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }
    while (num != 0) {
        int rem = num % base;
        buffer[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / 10;
    }
    if (is_negative) {
        buffer[i++] = '-';
    }
    buffer[i] = '\0';
    reverse_string(buffer, i);
    return buffer;
}

int simple_atoi(const char* str) {
    int res = 0;
    int sign = 1;
    int i = 0;
    if (str[0] == '-') {
        sign = -1;
        i++;
    }
    while(str[i] == ' ') i++;
    for (; str[i] != '\0'; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            res = res * 10 + str[i] - '0';
        } else {
            break;
        }
    }
    return res * sign;
}
 
// --- Fonctions de gestion de l'historique simplifiées ---
void add_to_history(const char* command) {
    size_t cmd_len = strlen(command);
    if (cmd_len == 0 || cmd_len >= MAX_COMMAND_LENGTH) return;
    
    // Éviter les doublons consécutifs
    if (history_count > 0) {
        int last_index = (history_count - 1) % MAX_HISTORY_SIZE;
        if (strcmp(history_commands[last_index], command) == 0) {
            return; // Ne pas ajouter un doublon
        }
    }
    
    // Ajouter la commande à l'historique avec protection
    int index = history_count % MAX_HISTORY_SIZE;
    
    // Copie sécurisée pour éviter le débordement
    size_t copy_len = cmd_len < MAX_COMMAND_LENGTH - 1 ? cmd_len : MAX_COMMAND_LENGTH - 1;
    for (size_t i = 0; i < copy_len; i++) {
        history_commands[index][i] = command[i];
    }
    history_commands[index][copy_len] = '\0';
    
    history_count++;
    history_index = history_count; // Réinitialiser l'index de navigation
}

const char* get_history_command(int direction) {
    if (history_count == 0) return NULL;
    
    // Direction -1 = flèche haut (remonter), +1 = flèche bas (descendre)
    if (direction == -1) {
        if (history_index > 0) {
            history_index--;
        }
    } else if (direction == 1) {
        if (history_index < history_count) {
            history_index++;
        }
    }
    
    if (history_index >= history_count) {
        return ""; // Ligne vide si on dépasse
    } else {
        int actual_index = history_index % MAX_HISTORY_SIZE;
        // Protection supplémentaire contre les accès hors limites
        if (actual_index >= 0 && actual_index < MAX_HISTORY_SIZE && history_index < history_count) {
            return history_commands[actual_index];
        }
    }
    return "";
}

void reset_history_navigation() {
    history_index = history_count;
}

// --- Fonctions des commandes ---
void do_calc(char* input) {
    if (!input || *input == '\0') {
        terminal_writestring("Usage: calc <nombre> <op> <nombre>\n");
        return;
    }
    int num1 = simple_atoi(input);
    char* op = input;
    if (*op == '-') op++;
    while (*op && (*op >= '0' && *op <= '9')) op++;
    while (*op == ' ') op++;
    char operator = *op;
    op++;
    int num2 = simple_atoi(op);
    int result = 0;
    switch (operator) {
        case '+': result = num1 + num2; break;
        case '-': result = num1 - num2; break;
        case '*': result = num1 * num2; break;
        case '/':
            if (num2 == 0) { terminal_writestring("Error: Division by zero!\n"); return; }
            result = num1 / num2;
            break;
        default: terminal_writestring("Error: Unknown operator.\n"); return;
    }
    char result_buffer[32];
    simple_itoa(result, result_buffer, 10);
    terminal_writestring(result_buffer);
    terminal_putchar('\n');
}

void do_chrono() {
    terminal_writestring("Chronometre demarre. Appuyez sur une touche pour arreter.\n");
    uint32_t start_tick = tick;
    getkey();
    terminal_putchar('\n');
    uint32_t end_tick = tick;
    uint32_t diff = end_tick - start_tick;
    uint32_t seconds = diff / 100;
    uint32_t centiseconds = diff % 100;
    char buffer[32];
    terminal_writestring("Temps ecoule : ");
    simple_itoa(seconds, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring(".");
    if (centiseconds < 10) {
        terminal_putchar('0');
    }
    simple_itoa(centiseconds, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring("s\n");
}

typedef struct {
    const char* name;
    uint8_t code;
} ColorMap;

static const ColorMap color_map[] = {
    {"black", 0x0}, {"blue", 0x1}, {"green", 0x2}, {"cyan", 0x3},
    {"red", 0x4}, {"magenta", 0x5}, {"brown", 0x6}, {"light_grey", 0x7},
    {"dark_grey", 0x8}, {"light_blue", 0x9}, {"light_green", 0xA},
    {"light_cyan", 0xB}, {"light_red", 0xC}, {"light_magenta", 0xD},
    {"yellow", 0xE}, {"white", 0xF}
};

int parse_color_string(const char* s) {
    if (s[0] >= '0' && s[0] <= '9' && s[1] == '\0') return s[0] - '0';
    if (s[0] >= 'a' && s[0] <= 'f' && s[1] == '\0') return s[0] - 'a' + 10;
    if (s[0] >= 'A' && s[0] <= 'F' && s[1] == '\0') return s[0] - 'A' + 10;
    for (size_t i = 0; i < sizeof(color_map) / sizeof(ColorMap); i++) {
        if (strcmp(s, color_map[i].name) == 0) {
            return color_map[i].code;
        }
    }
    return -1;
}

void do_color(char* args) {
    if (!args || strcmp(args, "help") == 0) {
        terminal_writestring("Usage: color <fg> [bg]\nEx: color yellow blue, color C 1\n");
        terminal_writestring("Colors: black, blue, green, cyan, red, magenta, brown, light_grey, dark_grey, light_blue, light_green, light_cyan, light_red, light_magenta, yellow, white.\n");
        return;
    }
    char* fg_str = args;
    char* bg_str = NULL;
    int i = 0;
    while (args[i] != ' ' && args[i] != '\0') i++;
    if (args[i] == ' ') {
        args[i] = '\0';
        bg_str = &args[i + 1];
    }
    int fg_code = parse_color_string(fg_str);
    if (fg_code == -1) {
        terminal_writestring("Invalid foreground color.\n");
        return;
    }
    int bg_code = -1;
    if (bg_str) {
        bg_code = parse_color_string(bg_str);
        if (bg_code == -1) {
            terminal_writestring("Invalid background color.\n");
            return;
        }
    }
    uint8_t current_bg = (terminal_color >> 4) & 0x0F;
    if (bg_code != -1) {
        current_bg = bg_code;
    }
    terminal_color = (current_bg << 4) | fg_code;
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = (vga_buffer[y * VGA_WIDTH + x] & 0x00FF) | (terminal_color << 8);
        }
    }
}

void display_boot_logo() {
    terminal_writestring("      M   M  Y   Y   OOO    SSS\n");
    terminal_writestring("      MM MM   Y Y   O   O  S   \n");
    terminal_writestring("      M M M    Y    O   O   SSS\n");
    terminal_writestring("      M   M    Y    O   O      S\n");
    terminal_writestring("      M   M    Y     OOO    SSS\n");
    terminal_writestring("\n");
    terminal_writestring("      ===========================\n");
    terminal_writestring("        Systeme d'Exploitation   \n");
    terminal_writestring("           Color Edition v1.1    \n");
    terminal_writestring("      ===========================\n");
    terminal_writestring("\n");
}

void do_charset() {
    terminal_writestring("Caracteres ASCII pour motifs de fond:\n");
    terminal_writestring("Symboles de base: ! \" # $ % & ' ( ) * + , - . /\n");
    terminal_writestring("Chiffres:           0 1 2 3 4 5 6 7 8 9\n");
    terminal_writestring("Lettres:            A-Z a-z\n");
    terminal_writestring("Autres symboles:    : ; < = > ? @ [ \\ ] ^ _ ` { | } ~\n");
    terminal_writestring("\nCaracteres pour motifs:\n");

    terminal_writestring("Blocs et points:  # * + . : ; = - _ | \\ /\n");
    terminal_writestring("Bordures:           + - | \\ / < > ^ v\n");
    terminal_writestring("Espacement:         . : ; , ' ` ~\n");

    terminal_writestring("\nExemples de motifs ASCII:\n");
    terminal_writestring("Grille: + - |\n");
    terminal_writestring("Points: . : ; ,\n");
    terminal_writestring("Lignes: - _ = ~ ^\n");
    terminal_writestring("Blocs:  # * @ %\n");

    terminal_writestring("\nUtilisez 'background' pour appliquer un motif!\n");
}

void draw_background_pattern() {
    // Sauvegarde du curseur actuel
    int old_x = cursor_x, old_y = cursor_y;

    // Motif de fond avec des caractères ASCII légers
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            char pattern_char = ' ';
            uint8_t pattern_color = 0x00; // Noir sur noir par défaut

            // Créer un motif géométrique ASCII
            if ((x + y) % 8 == 0) {
                pattern_char = '.'; // Point ASCII
                pattern_color = 0x08; // Gris foncé
            } else if (x % 10 == 0) {
                pattern_char = '|'; // Ligne verticale ASCII
                pattern_color = 0x08;
            } else if (y % 5 == 0) {
                pattern_char = '-'; // Ligne horizontale ASCII
                pattern_color = 0x08;
            }

            vga_buffer[y * VGA_WIDTH + x] = ((uint16_t)pattern_color << 8) | pattern_char;
        }
    }

    // Restaurer le curseur
    cursor_x = old_x;
    cursor_y = old_y;
    move_cursor(cursor_x, cursor_y);
}

void do_background(char* args) {
    if (!args) {
        terminal_writestring("Usage: background <pattern>\n");
        terminal_writestring("Patterns: clear, dots, lines, grid, stars\n");
        return;
    }

    if (strcmp(args, "clear") == 0) {
        clear_screen();
        terminal_writestring("Fond efface.\n");
    } else if (strcmp(args, "dots") == 0) {
        draw_background_pattern();
        cursor_x = 0; cursor_y = 0;
        move_cursor(cursor_x, cursor_y);
        terminal_writestring("Motif de points ASCII applique!\n");
    } else if (strcmp(args, "stars") == 0) {
        // Nouveau motif d'étoiles ASCII
        for (int y = 0; y < VGA_HEIGHT; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                if ((x * 7 + y * 3) % 23 == 0) {
                    vga_buffer[y * VGA_WIDTH + x] = (0x07 << 8) | '*'; // Étoiles blanches
                } else if ((x * 5 + y * 11) % 17 == 0) {
                    vga_buffer[y * VGA_WIDTH + x] = (0x08 << 8) | '.'; // Points gris
                } else {
                    vga_buffer[y * VGA_WIDTH + x] = (0x00 << 8) | ' ';
                }
            }
        }
        cursor_x = 0; cursor_y = 0;
        move_cursor(cursor_x, cursor_y);
        terminal_writestring("Motif d'etoiles applique!\n");
    } else if (strcmp(args, "lines") == 0) {
        // Motif de lignes ASCII
        for (int y = 0; y < VGA_HEIGHT; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                if (y % 2 == 0 && x % 4 == 0) {
                    vga_buffer[y * VGA_WIDTH + x] = (0x08 << 8) | '-'; // Ligne horizontale ASCII
                } else {
                    vga_buffer[y * VGA_WIDTH + x] = (0x00 << 8) | ' ';
                }
            }
        }
        cursor_x = 0; cursor_y = 0;
        move_cursor(cursor_x, cursor_y);
        terminal_writestring("Motif de lignes ASCII applique!\n");
    } else if (strcmp(args, "grid") == 0) {
        // Motif de grille ASCII
        for (int y = 0; y < VGA_HEIGHT; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                if ((x % 10 == 0 && y % 5 != 0) || (y % 5 == 0 && x % 10 != 0)) {
                    char grid_char = (x % 10 == 0) ? '|' : '-'; // Vertical ou horizontal ASCII
                    vga_buffer[y * VGA_WIDTH + x] = (0x08 << 8) | grid_char;
                } else if (x % 10 == 0 && y % 5 == 0) {
                    vga_buffer[y * VGA_WIDTH + x] = (0x08 << 8) | '+'; // Intersection ASCII
                } else {
                    vga_buffer[y * VGA_WIDTH + x] = (0x00 << 8) | ' ';
                }
            }
        }
        cursor_x = 0; cursor_y = 0;
        move_cursor(cursor_x, cursor_y);
        terminal_writestring("Motif de grille ASCII applique!\n");
    } else {
        terminal_writestring("Pattern inconnu. Utilisez: clear, dots, lines, grid, stars\n");
    }
}

void do_memory() {
    print_memory_stats();
}

void do_history() {
    terminal_writestring("=== HISTORIQUE ===\n");
    
    if (history_count == 0) {
        terminal_writestring("Aucune commande.\n");
        return;
    }
    
    for (int i = 0; i < history_count; i++) {
        char num_str[5];
        simple_itoa(i + 1, num_str, 10);
        terminal_writestring(num_str);
        terminal_writestring(". ");
        terminal_writestring(history_commands[i]);
        terminal_writestring("\n");
    }
}

void execute_command(char* line) {
    // Ajouter la commande à l'historique
    add_to_history(line);
    
    char* command = line;
    char* args = NULL;
    int i = 0;
    while (line[i] != ' ' && line[i] != '\0') i++;
    if (line[i] == ' ') {
        line[i] = '\0';
        args = &line[i + 1];
    }
    if (strcmp(command, "help") == 0) {
        terminal_writestring("Commands: help, clear, about, calc, chrono, snake, pong, color, blackjack, charset, background, memory, edit, files, history, password, wiki, chat, messages\n");
    } else if (strcmp(command, "clear") == 0) {
        clear_screen();
    } else if (strcmp(command, "about") == 0) {
        terminal_writestring("NOVA - v1.1 Color Edition\n");
    } else if (strcmp(command, "calc") == 0) {
        do_calc(args);
    } else if (strcmp(command, "chrono") == 0) {
        do_chrono();
    } else if (strcmp(command, "snake") == 0) {
        play_snake();
        clear_screen();
        terminal_writestring("Welcome back to the shell!\n");
    } else if (strcmp(command, "pong") == 0) {
        play_pong();
        clear_screen();
        terminal_writestring("Welcome back to the shell!\n");
    } else if (strcmp(command, "color") == 0) {
        do_color(args);
    } else if (strcmp(command, "charset") == 0) {
        do_charset();
    } else if (strcmp(command, "background") == 0) {
        do_background(args);
    } else if (strcmp(command, "memory") == 0) {
        do_memory();
    } else if (strcmp(command, "edit") == 0) {
        run_text_editor(args);
    } else if (strcmp(command, "files") == 0) {
        run_file_manager();
    } else if (strcmp(command, "history") == 0) {
        do_history();
    } else if (strcmp(command, "password") == 0) {
        run_password_generator(args);
    } else if (strcmp(command, "wiki") == 0) {
        do_wiki(args);
    } else if (strcmp(command, "chat") == 0 || strcmp(command, "messages") == 0) {
        do_messaging();
    } else if (strcmp(command, "blackjack") == 0) {
        if (play_blackjack()) {
            // Le joueur a gagné au blackjack, accès autorisé à Health
            health_app();
            clear_screen();
            terminal_writestring("Retour au shell principal.\n");
        } else {
            // Le joueur a perdu, retour au shell sans accès
            clear_screen();
            terminal_writestring("Acces aux dossiers medicaux refuse!\n");
            terminal_writestring("Gagnez au blackjack pour y acceder.\n");
        }
    } else if (strcmp(command, "medic_admin") == 0) {
        // Commande secrète pour accès direct (pour les tests)
        terminal_writestring("=== ACCES ADMINISTRATEUR ===\n");
        terminal_writestring("Acces direct aux dossiers medicaux autorise.\n\n");
        health_app();
        clear_screen();
        terminal_writestring("Retour au shell principal.\n");
    } else if (command[0] != '\0') {
        terminal_writestring("Unknown command.\n");
    }
}

// --- Point d'entrée principal du Kernel ---
void kernel_main(void) {
    gdt_install();
    idt_install();
    timer_install();
    memory_init();
    
    // Initialiser les nouveaux systèmes
    messaging_init();
    wiki_init();
    
    asm volatile("sti");

    clear_screen();
    display_boot_logo();
    terminal_writestring("Welcome to NOVA v1.1 - Color Edition with Messaging & Wiki!\n\n");

    char command_buffer[128];
    while (1) {
        // Vérifier les messages automatiques
        messaging_check_auto_messages();
        
        // Afficher le prompt avec notifications
        messaging_display_prompt_with_notifications();
        
        readline(command_buffer, 128);
        execute_command(command_buffer);
    }
}
