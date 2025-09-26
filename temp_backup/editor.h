#ifndef EDITOR_H
#define EDITOR_H

#include <stdint.h>

// Constantes pour l'éditeur
#define MAX_LINES 1000
#define MAX_LINE_LENGTH 80
#define MAX_FILENAME_LENGTH 64

// Structure pour une ligne de texte
typedef struct {
    char content[MAX_LINE_LENGTH];
    int length;
} text_line_t;

// Structure pour le document
typedef struct {
    text_line_t lines[MAX_LINES];
    int line_count;
    int cursor_x;
    int cursor_y;
    char filename[MAX_FILENAME_LENGTH];
    int modified;
    int insert_mode; // 0 = overwrite, 1 = insert
} editor_t;

// Fonctions principales de l'éditeur
void editor_init(editor_t* editor);
void editor_run(editor_t* editor);
void editor_display(editor_t* editor);
void editor_handle_input(editor_t* editor, int key);

// Fonctions de navigation
void editor_move_cursor(editor_t* editor, int dx, int dy);
void editor_move_to_line_start(editor_t* editor);
void editor_move_to_line_end(editor_t* editor);
void editor_move_to_document_start(editor_t* editor);
void editor_move_to_document_end(editor_t* editor);

// Fonctions d'édition
void editor_insert_char(editor_t* editor, char c);
void editor_delete_char(editor_t* editor);
void editor_backspace(editor_t* editor);
void editor_insert_line(editor_t* editor);
void editor_delete_line(editor_t* editor);
void editor_split_line(editor_t* editor);

// Fonctions de fichier
int editor_save_file(editor_t* editor);
int editor_load_file(editor_t* editor, const char* filename);
void editor_new_file(editor_t* editor);

// Fonctions utilitaires
void editor_clear_line(int y);
void editor_display_status_bar(editor_t* editor);
void editor_display_help(void);
void editor_show_message(const char* message);

// Fonction principale accessible depuis le shell
void run_text_editor(const char* filename);

#endif // EDITOR_H


