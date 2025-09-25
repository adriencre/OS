#ifndef PASSWORD_H
#define PASSWORD_H

#include <stdint.h>

// Constantes pour la génération de mots de passe
#define MAX_PASSWORD_LENGTH 64
#define MIN_PASSWORD_LENGTH 4
#define DEFAULT_PASSWORD_LENGTH 12

// Types de caractères disponibles
#define PASSWORD_LOWERCASE 0x01
#define PASSWORD_UPPERCASE 0x02
#define PASSWORD_DIGITS    0x04
#define PASSWORD_SYMBOLS   0x08
#define PASSWORD_ALL       0x0F

// Structure pour la configuration du générateur
typedef struct {
    int length;
    uint8_t char_types;
    int exclude_similar;  // Exclure les caractères similaires (0, O, l, 1, etc.)
    int exclude_ambiguous; // Exclure les caractères ambigus
} password_config_t;

// Fonctions principales
void password_generator_init(void);
void generate_password(char* buffer, const password_config_t* config);
void generate_multiple_passwords(int count, const password_config_t* config);
void show_password_strength(const char* password);
void show_password_help(void);

// Fonctions utilitaires
int is_password_strong(const char* password);
int count_char_types(const char* password);
void shuffle_string(char* str, int length);

// Fonction principale accessible depuis le shell
void run_password_generator(const char* args);

#endif // PASSWORD_H
