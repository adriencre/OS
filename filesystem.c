#include "filesystem.h"
#include "common.h"

// Variables globales
filesystem_t g_filesystem;

// Fonctions externes du kernel
extern int strlen(const char* str);
extern char* strcpy(char* dest, const char* src);
extern int strcmp(const char* s1, const char* s2);
extern char* strcat(char* dest, const char* src);

// Fonction strcat simple
char* strcat(char* dest, const char* src) {
    int dest_len = strlen(dest);
    int i = 0;
    while (src[i] != '\0') {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
    return dest;
}

// Initialisation du système de fichiers
void fs_init(filesystem_t* fs) {
    fs->num_files = 0;
    fs->current_dir = -1;
    fs->root_dir = -1;
    
    // Créer le répertoire racine
    fs->root_dir = fs_create_directory(fs, "/", "/");
    fs->current_dir = fs->root_dir;
    
    // Créer quelques fichiers et répertoires d'exemple
    fs_create_directory(fs, "home", "/home");
    fs_create_directory(fs, "documents", "/home/documents");
    fs_create_directory(fs, "images", "/home/images");
    
    // Créer des fichiers d'exemple
    int readme = fs_create_file(fs, "readme.txt", "/home/readme.txt", FILE_TYPE_TEXT);
    if (readme >= 0) {
        fs_write_file(fs, readme, "Bienvenue dans MyOS!\nCeci est un fichier de demonstration.\n");
    }
    
    int config = fs_create_file(fs, "config.ini", "/home/config.ini", FILE_TYPE_TEXT);
    if (config >= 0) {
        fs_write_file(fs, config, "[system]\nversion=1.0\nmode=text\n");
    }
    
    int hidden = fs_create_file(fs, ".hidden", "/home/.hidden", FILE_TYPE_TEXT);
    if (hidden >= 0) {
        fs_write_file(fs, hidden, "Ce fichier est cache.\n");
        fs->files[hidden].is_hidden = 1;
    }
    
    int app = fs_create_file(fs, "myapp", "/home/myapp", FILE_TYPE_EXECUTABLE);
    if (app >= 0) {
        fs_write_file(fs, app, "#!/bin/sh\necho 'Hello from MyOS!'\n");
    }
}

// Créer un fichier
int fs_create_file(filesystem_t* fs, const char* name, const char* path, file_type_t type) {
    if (fs->num_files >= MAX_FILES) return -1;
    if (!fs_is_valid_name(name)) return -1;
    
    // Vérifier si le fichier existe déjà
    for (int i = 0; i < fs->num_files; i++) {
        if (strcmp(fs->files[i].name, name) == 0 && fs->files[i].parent_dir == fs->current_dir) {
            return -1; // Fichier existe déjà
        }
    }
    
    int index = fs->num_files++;
    file_entry_t* file = &fs->files[index];
    
    strcpy(file->name, name);
    strcpy(file->path, path);
    file->type = type;
    file->size = 0;
    file->permissions = 0x06; // rw-
    file->is_directory = 0;
    file->is_hidden = 0;
    file->content[0] = '\0';
    file->parent_dir = fs->current_dir;
    file->num_children = 0;
    
    // Ajouter à la liste des enfants du répertoire parent
    if (fs->current_dir >= 0) {
        file_entry_t* parent = &fs->files[fs->current_dir];
        if (parent->num_children < MAX_FILES) {
            parent->children[parent->num_children++] = index;
        }
    }
    
    return index;
}

// Créer un répertoire
int fs_create_directory(filesystem_t* fs, const char* name, const char* path) {
    int index = fs_create_file(fs, name, path, FILE_TYPE_DIRECTORY);
    if (index >= 0) {
        fs->files[index].is_directory = 1;
        fs->files[index].permissions = 0x07; // rwx
    }
    return index;
}

// Supprimer un fichier
int fs_delete_file(filesystem_t* fs, int file_index) {
    if (file_index < 0 || file_index >= fs->num_files) return -1;
    
    file_entry_t* file = &fs->files[file_index];
    
    // Si c'est un répertoire, supprimer tous les enfants
    if (file->is_directory) {
        for (int i = 0; i < file->num_children; i++) {
            fs_delete_file(fs, file->children[i]);
        }
    }
    
    // Retirer du répertoire parent
    if (file->parent_dir >= 0) {
        file_entry_t* parent = &fs->files[file->parent_dir];
        for (int i = 0; i < parent->num_children; i++) {
            if (parent->children[i] == file_index) {
                // Décaler les éléments
                for (int j = i; j < parent->num_children - 1; j++) {
                    parent->children[j] = parent->children[j + 1];
                }
                parent->num_children--;
                break;
            }
        }
    }
    
    // Décaler tous les fichiers après celui-ci
    for (int i = file_index; i < fs->num_files - 1; i++) {
        fs->files[i] = fs->files[i + 1];
    }
    fs->num_files--;
    
    // Mettre à jour les références
    for (int i = 0; i < fs->num_files; i++) {
        if (fs->files[i].parent_dir > file_index) {
            fs->files[i].parent_dir--;
        }
        for (int j = 0; j < fs->files[i].num_children; j++) {
            if (fs->files[i].children[j] > file_index) {
                fs->files[i].children[j]--;
            }
        }
    }
    
    return 0;
}

// Renommer un fichier
int fs_rename_file(filesystem_t* fs, int file_index, const char* new_name) {
    if (file_index < 0 || file_index >= fs->num_files) return -1;
    if (!fs_is_valid_name(new_name)) return -1;
    
    // Vérifier si le nouveau nom existe déjà
    file_entry_t* file = &fs->files[file_index];
    for (int i = 0; i < fs->num_files; i++) {
        if (i != file_index && strcmp(fs->files[i].name, new_name) == 0 && 
            fs->files[i].parent_dir == file->parent_dir) {
            return -1; // Nom existe déjà
        }
    }
    
    strcpy(file->name, new_name);
    return 0;
}

// Écrire dans un fichier
int fs_write_file(filesystem_t* fs, int file_index, const char* content) {
    if (file_index < 0 || file_index >= fs->num_files) return -1;
    
    file_entry_t* file = &fs->files[file_index];
    if (file->is_directory) return -1; // Ne peut pas écrire dans un répertoire
    
    int len = strlen(content);
    if (len >= MAX_FILE_SIZE) len = MAX_FILE_SIZE - 1;
    
    strcpy(file->content, content);
    file->content[len] = '\0';
    file->size = len;
    
    return 0;
}

// Lire un fichier
const char* fs_read_file(filesystem_t* fs, int file_index) {
    if (file_index < 0 || file_index >= fs->num_files) return NULL;
    
    file_entry_t* file = &fs->files[file_index];
    if (file->is_directory) return NULL; // Ne peut pas lire un répertoire
    
    return file->content;
}

// Trouver un fichier par nom
int fs_find_file(filesystem_t* fs, const char* name) {
    for (int i = 0; i < fs->num_files; i++) {
        if (strcmp(fs->files[i].name, name) == 0 && fs->files[i].parent_dir == fs->current_dir) {
            return i;
        }
    }
    return -1;
}

// Lister le contenu d'un répertoire
int fs_list_directory(filesystem_t* fs, int dir_index, int* file_list, int max_files) {
    if (dir_index < 0 || dir_index >= fs->num_files) return -1;
    
    file_entry_t* dir = &fs->files[dir_index];
    if (!dir->is_directory) return -1;
    
    int count = 0;
    for (int i = 0; i < dir->num_children && count < max_files; i++) {
        file_list[count++] = dir->children[i];
    }
    
    return count;
}

// Changer de répertoire
int fs_change_directory(filesystem_t* fs, const char* path) {
    if (strcmp(path, "/") == 0) {
        fs->current_dir = fs->root_dir;
        return 0;
    }
    
    if (strcmp(path, "..") == 0) {
        if (fs->current_dir >= 0 && fs->files[fs->current_dir].parent_dir >= 0) {
            fs->current_dir = fs->files[fs->current_dir].parent_dir;
            return 0;
        }
        return -1;
    }
    
    // Chercher le répertoire
    for (int i = 0; i < fs->num_files; i++) {
        if (strcmp(fs->files[i].name, path) == 0 && 
            fs->files[i].is_directory && 
            fs->files[i].parent_dir == fs->current_dir) {
            fs->current_dir = i;
            return 0;
        }
    }
    
    return -1;
}

// Obtenir le répertoire courant
int fs_get_current_directory(filesystem_t* fs, char* path, int max_len) {
    if (fs->current_dir < 0) {
        strcpy(path, "/");
        return 0;
    }
    
    strcpy(path, fs->files[fs->current_dir].path);
    return 0;
}

// Déterminer le type de fichier par extension
file_type_t fs_get_file_type(const char* filename) {
    int len = strlen(filename);
    if (len < 2) return FILE_TYPE_UNKNOWN;
    
    // Chercher l'extension
    for (int i = len - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            const char* ext = &filename[i + 1];
            if (strcmp(ext, "txt") == 0 || strcmp(ext, "md") == 0 || strcmp(ext, "ini") == 0) {
                return FILE_TYPE_TEXT;
            } else if (strcmp(ext, "bin") == 0 || strcmp(ext, "exe") == 0) {
                return FILE_TYPE_BINARY;
            }
            break;
        }
    }
    
    // Vérifier si c'est un exécutable
    if (filename[0] != '.' && strcmp(filename, "myapp") == 0) {
        return FILE_TYPE_EXECUTABLE;
    }
    
    return FILE_TYPE_UNKNOWN;
}

// Vérifier si un nom de fichier est valide
int fs_is_valid_name(const char* name) {
    if (!name || strlen(name) == 0 || strlen(name) >= MAX_FILENAME_LENGTH) return 0;
    
    // Vérifier les caractères interdits
    for (int i = 0; name[i]; i++) {
        if (name[i] == '/' || name[i] == '\\' || name[i] == ':' || 
            name[i] == '*' || name[i] == '?' || name[i] == '"' || 
            name[i] == '<' || name[i] == '>' || name[i] == '|') {
            return 0;
        }
    }
    
    return 1;
}

// Obtenir les informations d'un fichier
void fs_get_file_info(filesystem_t* fs, int file_index, char* info, int max_len) {
    if (file_index < 0 || file_index >= fs->num_files) {
        strcpy(info, "Fichier introuvable");
        return;
    }
    
    file_entry_t* file = &fs->files[file_index];
    char size_str[16];
    
    strcpy(info, "Nom: ");
    strcat(info, file->name);
    strcat(info, "\nType: ");
    
    switch(file->type) {
        case FILE_TYPE_TEXT: strcat(info, "Texte"); break;
        case FILE_TYPE_BINARY: strcat(info, "Binaire"); break;
        case FILE_TYPE_EXECUTABLE: strcat(info, "Executable"); break;
        case FILE_TYPE_DIRECTORY: strcat(info, "Repertoire"); break;
        default: strcat(info, "Inconnu"); break;
    }
    
    strcat(info, "\nTaille: ");
    if (file->is_directory) {
        strcat(info, "-");
    } else {
        // Convertir la taille en chaîne (simulation)
        strcat(info, "bytes");
    }
    
    strcat(info, "\nPermissions: ");
    if (file->permissions & 0x04) strcat(info, "r"); else strcat(info, "-");
    if (file->permissions & 0x02) strcat(info, "w"); else strcat(info, "-");
    if (file->permissions & 0x01) strcat(info, "x"); else strcat(info, "-");
}


