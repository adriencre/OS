#include "common.h"
#include "editor.h"
#include "keyboard.h"
#include "timer.h"

// Déclarations externes du kernel
extern volatile uint16_t* vga_buffer;
extern const int VGA_WIDTH;
extern const int VGA_HEIGHT;
extern uint8_t terminal_color;
extern void terminal_putentryat(char c, uint8_t color, int x, int y);
extern void clear_screen();
extern void move_cursor(int x, int y);
extern void terminal_writestring(const char* data);
extern char* simple_itoa(int num, char* buffer, int base);
extern int strlen(const char* str);
extern char* strcpy(char* dest, const char* src);

// strcpy est maintenant définie dans kernel.c

// Variables globales pour l'éditeur
static editor_t g_editor;
static int editor_running = 0;

// Fonction pour effacer une ligne complète
void editor_clear_line(int y) {
    for (int x = 0; x < VGA_WIDTH; x++) {
        terminal_putentryat(' ', terminal_color, x, y);
    }
}

// Initialisation de l'éditeur
void editor_init(editor_t* editor) {
    editor->line_count = 1;
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->modified = 0;
    editor->insert_mode = 1; // Mode insertion par défaut
    
    // Initialiser la première ligne
    editor->lines[0].content[0] = '\0';
    editor->lines[0].length = 0;
    
    // Nom de fichier par défaut
    strcpy(editor->filename, "untitled.txt");
}

// Affichage du contenu de l'éditeur
void editor_display(editor_t* editor) {
    // Effacer l'écran
    clear_screen();
    
    // Afficher les lignes du document
    for (int i = 0; i < editor->line_count && i < VGA_HEIGHT - 2; i++) {
        editor_clear_line(i);
        for (int j = 0; j < editor->lines[i].length && j < VGA_WIDTH; j++) {
            terminal_putentryat(editor->lines[i].content[j], terminal_color, j, i);
        }
    }
    
    // Afficher la barre de statut
    editor_display_status_bar(editor);
    
    // Positionner le curseur
    move_cursor(editor->cursor_x, editor->cursor_y);
}

// Affichage de la barre de statut
void editor_display_status_bar(editor_t* editor) {
    int status_y = VGA_HEIGHT - 1;
    editor_clear_line(status_y);
    
    // Afficher les informations de base
    terminal_writestring("File: ");
    terminal_writestring(editor->filename);
    
    if (editor->modified) {
        terminal_writestring(" [MODIFIED]");
    }
    
    terminal_writestring(" | Line: ");
    char line_num[10];
    simple_itoa(editor->cursor_y + 1, line_num, 10);
    terminal_writestring(line_num);
    
    terminal_writestring(" Col: ");
    char col_num[10];
    simple_itoa(editor->cursor_x + 1, col_num, 10);
    terminal_writestring(col_num);
    
    if (editor->insert_mode) {
        terminal_writestring(" | INSERT");
    } else {
        terminal_writestring(" | OVERWRITE");
    }
}

// Affichage de l'aide
void editor_display_help(void) {
    clear_screen();
    terminal_writestring("=== EDITEUR DE TEXTE - AIDE ===\n\n");
    terminal_writestring("NAVIGATION:\n");
    terminal_writestring("  Fleches     - Deplacer le curseur\n");
    terminal_writestring("  Home        - Debut de ligne\n");
    terminal_writestring("  End         - Fin de ligne\n");
    terminal_writestring("  Ctrl+Home   - Debut du document\n");
    terminal_writestring("  Ctrl+End    - Fin du document\n\n");
    
    terminal_writestring("EDITION:\n");
    terminal_writestring("  Insert      - Basculer mode insertion/ecrasement\n");
    terminal_writestring("  Delete      - Supprimer caractere a droite\n");
    terminal_writestring("  Backspace   - Supprimer caractere a gauche\n");
    terminal_writestring("  Enter       - Nouvelle ligne\n");
    terminal_writestring("  Ctrl+D      - Supprimer ligne courante\n\n");
    
    terminal_writestring("FICHIERS:\n");
    terminal_writestring("  Ctrl+S      - Sauvegarder\n");
    terminal_writestring("  Ctrl+O      - Ouvrir fichier\n");
    terminal_writestring("  Ctrl+N      - Nouveau fichier\n\n");
    
    terminal_writestring("AUTRES:\n");
    terminal_writestring("  F1          - Afficher cette aide\n");
    terminal_writestring("  Escape      - Quitter l'editeur\n\n");
    
    terminal_writestring("Appuyez sur une touche pour continuer...");
    getkey();
}

// Affichage d'un message
void editor_show_message(const char* message) {
    int msg_y = VGA_HEIGHT - 2;
    editor_clear_line(msg_y);
    
    // Afficher le message centré
    move_cursor(0, msg_y);
    terminal_writestring(message);
}

// Déplacement du curseur
void editor_move_cursor(editor_t* editor, int dx, int dy) {
    int new_x = editor->cursor_x + dx;
    int new_y = editor->cursor_y + dy;
    
    // Limiter la position verticale
    if (new_y < 0) new_y = 0;
    if (new_y >= editor->line_count) new_y = editor->line_count - 1;
    
    // Limiter la position horizontale selon la ligne
    int max_x = editor->lines[new_y].length;
    if (new_x < 0) new_x = 0;
    if (new_x > max_x) new_x = max_x;
    
    editor->cursor_x = new_x;
    editor->cursor_y = new_y;
}

// Déplacement au début de ligne
void editor_move_to_line_start(editor_t* editor) {
    editor->cursor_x = 0;
}

// Déplacement à la fin de ligne
void editor_move_to_line_end(editor_t* editor) {
    editor->cursor_x = editor->lines[editor->cursor_y].length;
}

// Déplacement au début du document
void editor_move_to_document_start(editor_t* editor) {
    editor->cursor_x = 0;
    editor->cursor_y = 0;
}

// Déplacement à la fin du document
void editor_move_to_document_end(editor_t* editor) {
    editor->cursor_y = editor->line_count - 1;
    editor->cursor_x = editor->lines[editor->cursor_y].length;
}

// Insertion d'un caractère
void editor_insert_char(editor_t* editor, char c) {
    if (editor->cursor_y >= MAX_LINES) return;
    
    text_line_t* line = &editor->lines[editor->cursor_y];
    
    if (editor->insert_mode) {
        // Mode insertion : décaler les caractères
        if (line->length < MAX_LINE_LENGTH - 1) {
            for (int i = line->length; i > editor->cursor_x; i--) {
                line->content[i] = line->content[i - 1];
            }
            line->content[editor->cursor_x] = c;
            line->length++;
            line->content[line->length] = '\0';
        }
    } else {
        // Mode écrasement
        if (editor->cursor_x < MAX_LINE_LENGTH - 1) {
            line->content[editor->cursor_x] = c;
            if (editor->cursor_x >= line->length) {
                line->length = editor->cursor_x + 1;
                line->content[line->length] = '\0';
            }
        }
    }
    
    editor->cursor_x++;
    editor->modified = 1;
}

// Suppression d'un caractère (Delete)
void editor_delete_char(editor_t* editor) {
    text_line_t* line = &editor->lines[editor->cursor_y];
    
    if (editor->cursor_x < line->length) {
        for (int i = editor->cursor_x; i < line->length - 1; i++) {
            line->content[i] = line->content[i + 1];
        }
        line->length--;
        line->content[line->length] = '\0';
        editor->modified = 1;
    } else if (editor->cursor_y < editor->line_count - 1) {
        // Fusionner avec la ligne suivante
        text_line_t* next_line = &editor->lines[editor->cursor_y + 1];
        if (line->length + next_line->length < MAX_LINE_LENGTH) {
            for (int i = 0; i < next_line->length; i++) {
                line->content[line->length + i] = next_line->content[i];
            }
            line->length += next_line->length;
            line->content[line->length] = '\0';
            
            // Supprimer la ligne suivante
            for (int i = editor->cursor_y + 1; i < editor->line_count - 1; i++) {
                editor->lines[i] = editor->lines[i + 1];
            }
            editor->line_count--;
            editor->modified = 1;
        }
    }
}

// Suppression d'un caractère (Backspace)
void editor_backspace(editor_t* editor) {
    if (editor->cursor_x > 0) {
        editor->cursor_x--;
        editor_delete_char(editor);
    } else if (editor->cursor_y > 0) {
        // Fusionner avec la ligne précédente
        text_line_t* prev_line = &editor->lines[editor->cursor_y - 1];
        text_line_t* curr_line = &editor->lines[editor->cursor_y];
        
        if (prev_line->length + curr_line->length < MAX_LINE_LENGTH) {
            for (int i = 0; i < curr_line->length; i++) {
                prev_line->content[prev_line->length + i] = curr_line->content[i];
            }
            prev_line->length += curr_line->length;
            prev_line->content[prev_line->length] = '\0';
            
            // Supprimer la ligne courante
            for (int i = editor->cursor_y; i < editor->line_count - 1; i++) {
                editor->lines[i] = editor->lines[i + 1];
            }
            editor->line_count--;
            
            editor->cursor_x = prev_line->length - curr_line->length;
            editor->cursor_y--;
            editor->modified = 1;
        }
    }
}

// Insertion d'une nouvelle ligne
void editor_insert_line(editor_t* editor) {
    if (editor->line_count >= MAX_LINES) return;
    
    // Décaler les lignes vers le bas
    for (int i = editor->line_count; i > editor->cursor_y + 1; i--) {
        editor->lines[i] = editor->lines[i - 1];
    }
    
    // Diviser la ligne courante
    text_line_t* curr_line = &editor->lines[editor->cursor_y];
    text_line_t* new_line = &editor->lines[editor->cursor_y + 1];
    
    // Copier la partie après le curseur vers la nouvelle ligne
    int chars_to_move = curr_line->length - editor->cursor_x;
    for (int i = 0; i < chars_to_move; i++) {
        new_line->content[i] = curr_line->content[editor->cursor_x + i];
    }
    new_line->length = chars_to_move;
    new_line->content[new_line->length] = '\0';
    
    // Tronquer la ligne courante
    curr_line->length = editor->cursor_x;
    curr_line->content[curr_line->length] = '\0';
    
    editor->line_count++;
    editor->cursor_x = 0;
    editor->cursor_y++;
    editor->modified = 1;
}

// Suppression de la ligne courante
void editor_delete_line(editor_t* editor) {
    if (editor->line_count <= 1) return;
    
    // Décaler les lignes vers le haut
    for (int i = editor->cursor_y; i < editor->line_count - 1; i++) {
        editor->lines[i] = editor->lines[i + 1];
    }
    
    editor->line_count--;
    
    // Ajuster la position du curseur
    if (editor->cursor_y >= editor->line_count) {
        editor->cursor_y = editor->line_count - 1;
    }
    if (editor->cursor_x > editor->lines[editor->cursor_y].length) {
        editor->cursor_x = editor->lines[editor->cursor_y].length;
    }
    
    editor->modified = 1;
}

// Nouveau fichier
void editor_new_file(editor_t* editor) {
    if (editor->modified) {
        editor_show_message("Document non sauvegarde! Utilisez Ctrl+S pour sauvegarder.");
        return;
    }
    
    editor_init(editor);
    strcpy(editor->filename, "untitled.txt");
    editor_show_message("Nouveau fichier cree.");
}

// Sauvegarde du fichier (simulation - en réalité, on affiche juste un message)
int editor_save_file(editor_t* editor) {
    // Dans un vrai OS, on écrirait sur le disque
    // Ici, on simule juste la sauvegarde
    editor->modified = 0;
    editor_show_message("Fichier sauvegarde (simulation)");
    return 1;
}

// Chargement d'un fichier (simulation)
int editor_load_file(editor_t* editor, const char* filename) {
    // Dans un vrai OS, on lirait depuis le disque
    // Ici, on simule juste le chargement
    strcpy(editor->filename, filename);
    editor_show_message("Fichier charge (simulation)");
    return 1;
}

// Gestion des entrées clavier
void editor_handle_input(editor_t* editor, int key) {
    switch (key) {
        // Navigation
        case 0x4B00: // Flèche gauche
            editor_move_cursor(editor, -1, 0);
            break;
        case 0x4D00: // Flèche droite
            editor_move_cursor(editor, 1, 0);
            break;
        case 0x4800: // Flèche haut
            editor_move_cursor(editor, 0, -1);
            break;
        case 0x5000: // Flèche bas
            editor_move_cursor(editor, 0, 1);
            break;
        case 0x4700: // Home
            editor_move_to_line_start(editor);
            break;
        case 0x4F00: // End
            editor_move_to_line_end(editor);
            break;
        case 0x7700: // Ctrl+Home
            editor_move_to_document_start(editor);
            break;
        case 0x7500: // Ctrl+End
            editor_move_to_document_end(editor);
            break;
            
        // Édition
        case 0x5200: // Insert
            editor->insert_mode = !editor->insert_mode;
            break;
        case 0x5300: // Delete
            editor_delete_char(editor);
            break;
        case '\b': // Backspace
            editor_backspace(editor);
            break;
        case '\n': // Enter
            editor_insert_line(editor);
            break;
        case 0x2000: // Ctrl+D
            editor_delete_line(editor);
            break;
            
        // Fichiers
        case 0x1F00: // Ctrl+S
            editor_save_file(editor);
            break;
        case 0x1800: // Ctrl+O
            editor_show_message("Ouvrir fichier (non implemente)");
            break;
        case 0x3100: // Ctrl+N
            editor_new_file(editor);
            break;
            
        // Aide et sortie
        case 0x3B00: // F1
            editor_display_help();
            break;
        case 0x011B: // Escape
            editor_running = 0;
            break;
            
        // Caractères normaux
        default:
            if (key >= 32 && key <= 126) { // Caractères imprimables
                editor_insert_char(editor, (char)key);
            }
            break;
    }
}

// Fonction principale de l'éditeur
void editor_run(editor_t* editor) {
    editor_running = 1;
    editor_display(editor);
    
    while (editor_running) {
        int key = getkey();
        editor_handle_input(editor, key);
        editor_display(editor);
    }
}

// Fonction accessible depuis le shell
void run_text_editor(const char* filename) {
    editor_init(&g_editor);
    
    if (filename && strlen(filename) > 0) {
        strcpy(g_editor.filename, filename);
        editor_load_file(&g_editor, filename);
    }
    
    editor_run(&g_editor);
    
    // Retour au shell
    clear_screen();
    terminal_writestring("Retour au shell principal.\n");
}
