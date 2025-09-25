#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

// Constantes du système de fichiers
#define MAX_FILES 50
#define MAX_FILENAME_LENGTH 32
#define MAX_PATH_LENGTH 64
#define MAX_FILE_SIZE 4096
#define MAX_DIRS 20

// Types de fichiers
typedef enum {
    FILE_TYPE_UNKNOWN = 0,
    FILE_TYPE_TEXT = 1,
    FILE_TYPE_BINARY = 2,
    FILE_TYPE_DIRECTORY = 3,
    FILE_TYPE_EXECUTABLE = 4
} file_type_t;

// Structure pour un fichier
typedef struct {
    char name[MAX_FILENAME_LENGTH];
    char path[MAX_PATH_LENGTH];
    file_type_t type;
    uint32_t size;
    uint8_t permissions; // 0x07 = rwx
    int is_directory;
    int is_hidden;
    char content[MAX_FILE_SIZE]; // Contenu du fichier
    int parent_dir; // Index du répertoire parent (-1 pour racine)
    int children[MAX_FILES]; // Indices des fichiers enfants
    int num_children;
} file_entry_t;

// Structure pour le système de fichiers
typedef struct {
    file_entry_t files[MAX_FILES];
    int num_files;
    int current_dir; // Index du répertoire courant
    int root_dir; // Index du répertoire racine
} filesystem_t;

// Fonctions du système de fichiers
void fs_init(filesystem_t* fs);
int fs_create_file(filesystem_t* fs, const char* name, const char* path, file_type_t type);
int fs_create_directory(filesystem_t* fs, const char* name, const char* path);
int fs_delete_file(filesystem_t* fs, int file_index);
int fs_rename_file(filesystem_t* fs, int file_index, const char* new_name);
int fs_write_file(filesystem_t* fs, int file_index, const char* content);
const char* fs_read_file(filesystem_t* fs, int file_index);
int fs_find_file(filesystem_t* fs, const char* name);
int fs_list_directory(filesystem_t* fs, int dir_index, int* file_list, int max_files);
int fs_change_directory(filesystem_t* fs, const char* path);
int fs_get_current_directory(filesystem_t* fs, char* path, int max_len);

// Fonctions utilitaires
file_type_t fs_get_file_type(const char* filename);
int fs_is_valid_name(const char* name);
void fs_get_file_info(filesystem_t* fs, int file_index, char* info, int max_len);

// Variables globales
extern filesystem_t g_filesystem;

#endif // FILESYSTEM_H


