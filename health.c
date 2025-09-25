#include "health.h"
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

// --- Fonctions Utilitaires ---
// strlen est maintenant définie dans kernel.c
extern size_t strlen(const char* s);

void delay_simple(int milliseconds) {
    for (volatile int i = 0; i < milliseconds * 10000; i++);
}

// --- Fonctions d'Aide à l'Affichage ---
void print_at_color(const char* str, uint8_t color, int x, int y) {
    int i = 0;
    while (str[i] != '\0') {
        terminal_putentryat(str[i], color, x + i, y);
        i++;
    }
}

void draw_box(const char* title, int x, int y, int width, int height) {
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
    print_at_color(title, title_color, x + 2, y);
}

void clear_area(int x, int y, int width, int height) {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            terminal_putentryat(' ', VGA_COLOR(COLOR_BLACK, COLOR_BLACK), x + i, y + j);
        }
    }
}

// --- Base de Données ---
static const HealthRecord records[] = {
    {0, "John Doe", "1985-05-21", "O+", "Pollen", "2023-01-15"},
    {1, "Jane Smith", "1992-11-30", "A-", "Aucune", "2023-03-10"},
    {2, "Peter Jones", "1978-08-12", "B+", "Arachides", "2022-11-20"},
    {3, "Emily White", "2001-02-25", "AB+", "Poussiere", "2024-05-30"},
    {4, "Michael Brown", "1965-07-19", "O-", "Aucune", "2024-08-01"},
    {5, "Sarah Green", "1999-12-01", "A+", "Latex", "2024-09-11"},
};
static const int num_patients = sizeof(records) / sizeof(HealthRecord);
#define PATIENTS_PER_PAGE 5

// --- Fonctions de l'Application ---
void list_all_patients() {
    char id_buffer[12], page_buffer[32];
    uint8_t header_color = VGA_COLOR(COLOR_YELLOW, COLOR_BLACK);
    uint8_t data_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);
    int start_index = 0;
    int total_pages = (num_patients + PATIENTS_PER_PAGE - 1) / PATIENTS_PER_PAGE;

    while (1) {
        clear_area(0, 5, 80, 17);
        int current_page = (start_index / PATIENTS_PER_PAGE) + 1;

        print_at_color(" ID ", header_color, 4, 6);
        print_at_color("| Nom du Patient", header_color, 9, 6);
        print_at_color("----+-------------------------", header_color, 4, 7);

        int end_index = start_index + PATIENTS_PER_PAGE;
        if (end_index > num_patients) end_index = num_patients;

        for (int i = start_index; i < end_index; i++) {
            simple_itoa(records[i].id, id_buffer, 10);
            print_at_color(id_buffer, data_color, 5, 8 + (i - start_index));
            print_at_color("|", header_color, 9, 8 + (i - start_index));
            print_at_color(records[i].name, data_color, 11, 8 + (i - start_index));
        }

        simple_itoa(current_page, page_buffer, 10);
        print_at_color("Page ", header_color, 4, 15);
        print_at_color(page_buffer, data_color, 9, 15);
        simple_itoa(total_pages, page_buffer, 10);
        print_at_color(" / ", header_color, 9 + strlen(page_buffer) + 1, 15);
        print_at_color(page_buffer, data_color, 9 + strlen(page_buffer) + 4, 15);

        print_at_color("Suivant (s) | Precedent (p) | Quitter (q)", VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 4, 17);

        int key = getkey();
        if (key == 'q') break;
        if (key == 's' && (start_index + PATIENTS_PER_PAGE < num_patients)) start_index += PATIENTS_PER_PAGE;
        if (key == 'p' && (start_index - PATIENTS_PER_PAGE >= 0)) start_index -= PATIENTS_PER_PAGE;
    }
}

void show_patient_by_id(int id) {
    if (id < 0 || id >= num_patients) {
        print_at_color("Erreur : ID de patient introuvable.", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
        return;
    }
    const HealthRecord* p = &records[id];
    char id_buffer[12];

    draw_box(" Fiche Patient ", 20, 6, 40, 9);
    uint8_t label_color = VGA_COLOR(COLOR_CYAN, COLOR_BLACK);
    uint8_t data_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);

    simple_itoa(p->id, id_buffer, 10);
    print_at_color("ID.............: ", label_color, 22, 8); print_at_color(id_buffer, data_color, 40, 8);
    print_at_color("Nom............: ", label_color, 22, 9); print_at_color(p->name, data_color, 40, 9);
    print_at_color("Date Naissance.: ", label_color, 22, 10); print_at_color(p->birth_date, data_color, 40, 10);
    print_at_color("Groupe Sanguin.: ", label_color, 22, 11); print_at_color(p->blood_type, data_color, 40, 11);
    print_at_color("Allergies......: ", label_color, 22, 12); print_at_color(p->allergies, data_color, 40, 12);
    print_at_color("Derniere Visite: ", label_color, 22, 13); print_at_color(p->last_visit, data_color, 40, 13);
}

void display_help() {
    draw_box(" Aide ", 20, 6, 40, 7);
    uint8_t cmd_color = VGA_COLOR(COLOR_YELLOW, COLOR_BLACK);
    uint8_t desc_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);
    print_at_color("liste", cmd_color, 22, 8);
    print_at_color("- Voir tous les patients", desc_color, 32, 8);
    print_at_color("voir <id>", cmd_color, 22, 9);
    print_at_color("- Details d'un patient", desc_color, 32, 9);
    print_at_color("quitter", cmd_color, 22, 10);
    print_at_color("- Fermer l'application", desc_color, 32, 10);
    print_at_color("aide", cmd_color, 22, 11);
    print_at_color("- Afficher cette aide", desc_color, 32, 11);
}

void parse_health_command(char* line, char* command, char* args) {
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
void health_app() {
    clear_screen();
    uint8_t title_color = VGA_COLOR(COLOR_CYAN, COLOR_BLACK);
    const char* title = "=== Application Dossiers Medicaux ===";
    print_at_color(title, title_color, (80 - strlen(title)) / 2, 2);
    display_help();

    char input_buffer[128], command[64], args[64];

    while (1) {
        clear_area(0, 22, 80, 2);
        print_at_color("DossiersMedicaux> ", VGA_COLOR(COLOR_GREEN, COLOR_BLACK), 2, 23);
        readline(input_buffer, sizeof(input_buffer));

        parse_health_command(input_buffer, command, args);

        clear_area(0, 5, 80, 17);

        if (strcmp(command, "liste") == 0) {
            list_all_patients();
            // ✅ FIX: Nettoie la zone ET réaffiche l'aide après avoir quitté la liste.
            clear_area(0, 5, 80, 17);
            display_help();
        } else if (strcmp(command, "voir") == 0) {
            if (args[0] != '\0') {
                show_patient_by_id(simple_atoi(args));
            } else {
                print_at_color("Usage: voir <id>", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
            }
        } else if (strcmp(command, "quitter") == 0) {
            print_at_color("Fermeture de l'application...", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 2, 22);
            delay_simple(150);
            break;
        } else if (command[0] != '\0') {
            display_help();
            if (strcmp(command, "aide") != 0) {
                 print_at_color("Commande inconnue.", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 22);
            }
        } else {
            display_help();
        }
    }
}
