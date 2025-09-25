// password.c - Générateur de mots de passe sécurisé
#include "password.h"
#include "common.h"
#include "keyboard.h"

// Déclarations de fonctions du kernel
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void clear_screen();
char* simple_itoa(int num, char* buffer, int base);
int simple_atoi(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
extern uint32_t tick;

// Caractères disponibles pour la génération
static const char lowercase_chars[] = "abcdefghijklmnopqrstuvwxyz";
static const char uppercase_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char digit_chars[] = "0123456789";
static const char symbol_chars[] = "!@#$%^&*()_+-=[]{}|;:,.<>?";

// Caractères similaires à exclure
static const char similar_chars[] = "0OIl1|";
static const char ambiguous_chars[] = "{}[]()/\\'\"`~,;.<>";

// Variables globales
static uint32_t seed = 0;

// Fonction de génération de nombres pseudo-aléatoires simple
uint32_t simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

// Initialisation du générateur
void password_generator_init(void) {
    // Utiliser le tick comme graine pour la randomisation
    seed = tick;
}

// Vérifier si un caractère est dans une chaîne
int char_in_string(char c, const char* str) {
    while (*str) {
        if (*str == c) return 1;
        str++;
    }
    return 0;
}

// Générer un caractère aléatoire selon le type
char generate_random_char(uint8_t char_types, int exclude_similar, int exclude_ambiguous) {
    char available_chars[256];
    int count = 0;
    
    // Construire la liste des caractères disponibles
    if (char_types & PASSWORD_LOWERCASE) {
        for (int i = 0; lowercase_chars[i]; i++) {
            char c = lowercase_chars[i];
            if (exclude_similar && char_in_string(c, similar_chars)) continue;
            if (exclude_ambiguous && char_in_string(c, ambiguous_chars)) continue;
            available_chars[count++] = c;
        }
    }
    
    if (char_types & PASSWORD_UPPERCASE) {
        for (int i = 0; uppercase_chars[i]; i++) {
            char c = uppercase_chars[i];
            if (exclude_similar && char_in_string(c, similar_chars)) continue;
            if (exclude_ambiguous && char_in_string(c, ambiguous_chars)) continue;
            available_chars[count++] = c;
        }
    }
    
    if (char_types & PASSWORD_DIGITS) {
        for (int i = 0; digit_chars[i]; i++) {
            char c = digit_chars[i];
            if (exclude_similar && char_in_string(c, similar_chars)) continue;
            if (exclude_ambiguous && char_in_string(c, ambiguous_chars)) continue;
            available_chars[count++] = c;
        }
    }
    
    if (char_types & PASSWORD_SYMBOLS) {
        for (int i = 0; symbol_chars[i]; i++) {
            char c = symbol_chars[i];
            if (exclude_similar && char_in_string(c, similar_chars)) continue;
            if (exclude_ambiguous && char_in_string(c, ambiguous_chars)) continue;
            available_chars[count++] = c;
        }
    }
    
    if (count == 0) {
        // Fallback si aucun caractère disponible
        return 'a';
    }
    
    return available_chars[simple_rand() % count];
}

// Générer un mot de passe
void generate_password(char* buffer, const password_config_t* config) {
    if (!buffer || !config) return;
    
    int length = config->length;
    if (length < MIN_PASSWORD_LENGTH) length = MIN_PASSWORD_LENGTH;
    if (length > MAX_PASSWORD_LENGTH) length = MAX_PASSWORD_LENGTH;
    
    // Générer les caractères
    for (int i = 0; i < length; i++) {
        buffer[i] = generate_random_char(config->char_types, 
                                       config->exclude_similar, 
                                       config->exclude_ambiguous);
    }
    buffer[length] = '\0';
    
    // Mélanger le mot de passe pour plus de sécurité
    shuffle_string(buffer, length);
}

// Mélanger une chaîne (algorithme Fisher-Yates)
void shuffle_string(char* str, int length) {
    for (int i = length - 1; i > 0; i--) {
        int j = simple_rand() % (i + 1);
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

// Compter les types de caractères dans un mot de passe
int count_char_types(const char* password) {
    int has_lower = 0, has_upper = 0, has_digit = 0, has_symbol = 0;
    
    for (int i = 0; password[i]; i++) {
        char c = password[i];
        
        if (c >= 'a' && c <= 'z') has_lower = 1;
        else if (c >= 'A' && c <= 'Z') has_upper = 1;
        else if (c >= '0' && c <= '9') has_digit = 1;
        else has_symbol = 1;
    }
    
    return has_lower + has_upper + has_digit + has_symbol;
}

// Vérifier la force d'un mot de passe
int is_password_strong(const char* password) {
    if (!password) return 0;
    
    int len = 0;
    while (password[len]) len++;
    
    // Critères de force
    if (len < 8) return 0;  // Trop court
    if (count_char_types(password) < 3) return 0;  // Pas assez de variété
    
    return 1;
}

// Afficher l'analyse de force d'un mot de passe
void show_password_strength(const char* password) {
    if (!password) return;
    
    terminal_writestring("=== ANALYSE DE FORCE ===\n");
    
    int len = 0;
    while (password[len]) len++;
    
    terminal_writestring("Longueur: ");
    char len_str[10];
    simple_itoa(len, len_str, 10);
    terminal_writestring(len_str);
    terminal_writestring(" caracteres\n");
    
    int types = count_char_types(password);
    terminal_writestring("Types de caracteres: ");
    char types_str[5];
    simple_itoa(types, types_str, 10);
    terminal_writestring(types_str);
    terminal_writestring("/4\n");
    
    terminal_writestring("Force: ");
    if (is_password_strong(password)) {
        terminal_writestring("FORT");
    } else {
        terminal_writestring("FAIBLE");
    }
    terminal_writestring("\n");
    
    // Recommandations
    terminal_writestring("Recommandations:\n");
    if (len < 8) {
        terminal_writestring("- Augmenter la longueur (minimum 8)\n");
    }
    if (types < 3) {
        terminal_writestring("- Utiliser plus de types de caracteres\n");
    }
    if (types == 4) {
        terminal_writestring("- Excellent! Utilise tous les types\n");
    }
}

// Générer plusieurs mots de passe
void generate_multiple_passwords(int count, const password_config_t* config) {
    if (count <= 0 || count > 20) count = 5;  // Limite raisonnable
    
    terminal_writestring("=== GENERATION DE ");
    char count_str[5];
    simple_itoa(count, count_str, 10);
    terminal_writestring(count_str);
    terminal_writestring(" MOTS DE PASSE ===\n\n");
    
    for (int i = 0; i < count; i++) {
        char password[MAX_PASSWORD_LENGTH + 1];
        generate_password(password, config);
        
        terminal_writestring("Password ");
        char num_str[5];
        simple_itoa(i + 1, num_str, 10);
        terminal_writestring(num_str);
        terminal_writestring(": ");
        terminal_writestring(password);
        terminal_writestring("\n");
    }
}

// Afficher l'aide
void show_password_help(void) {
    terminal_writestring("=== GENERATEUR DE MOTS DE PASSE ===\n");
    terminal_writestring("Usage: password [options]\n\n");
    terminal_writestring("Options:\n");
    terminal_writestring("  -l <longueur>     Longueur du mot de passe (4-64)\n");
    terminal_writestring("  -t <types>        Types de caracteres (1-4)\n");
    terminal_writestring("                    1=minuscules, 2=majuscules, 3=chiffres, 4=symboles\n");
    terminal_writestring("  -n <nombre>       Generer plusieurs mots de passe\n");
    terminal_writestring("  -s                Exclure caracteres similaires (0,O,l,1,|)\n");
    terminal_writestring("  -a                Exclure caracteres ambigus\n");
    terminal_writestring("  -c <motdepasse>   Analyser la force d'un mot de passe\n");
    terminal_writestring("  -h                Afficher cette aide\n\n");
    terminal_writestring("Exemples:\n");
    terminal_writestring("  password                    # Mot de passe par defaut\n");
    terminal_writestring("  password -l 16              # Mot de passe de 16 caracteres\n");
    terminal_writestring("  password -t 15              # Minuscules + majuscules + chiffres\n");
    terminal_writestring("  password -n 5 -l 20         # 5 mots de passe de 20 caracteres\n");
    terminal_writestring("  password -s -a              # Exclure caracteres problematiques\n");
    terminal_writestring("  password -c MonMotDePasse   # Analyser un mot de passe\n");
}

// Parser les arguments de ligne de commande
void parse_password_args(const char* args, password_config_t* config) {
    // Configuration par défaut
    config->length = DEFAULT_PASSWORD_LENGTH;
    config->char_types = PASSWORD_ALL;
    config->exclude_similar = 0;
    config->exclude_ambiguous = 0;
    
    if (!args || *args == '\0') return;
    
    // Parser simple des arguments
    const char* p = args;
    while (*p) {
        if (*p == '-') {
            p++;
            switch (*p) {
                case 'l':  // Longueur
                    p++;
                    while (*p == ' ') p++;
                    if (*p >= '0' && *p <= '9') {
                        config->length = simple_atoi(p);
                        if (config->length < MIN_PASSWORD_LENGTH) config->length = MIN_PASSWORD_LENGTH;
                        if (config->length > MAX_PASSWORD_LENGTH) config->length = MAX_PASSWORD_LENGTH;
                    }
                    break;
                case 't':  // Types
                    p++;
                    while (*p == ' ') p++;
                    if (*p >= '0' && *p <= '9') {
                        int types = simple_atoi(p);
                        config->char_types = 0;
                        if (types & 1) config->char_types |= PASSWORD_LOWERCASE;
                        if (types & 2) config->char_types |= PASSWORD_UPPERCASE;
                        if (types & 4) config->char_types |= PASSWORD_DIGITS;
                        if (types & 8) config->char_types |= PASSWORD_SYMBOLS;
                        if (config->char_types == 0) config->char_types = PASSWORD_ALL;
                    }
                    break;
                case 's':  // Exclure similaires
                    config->exclude_similar = 1;
                    break;
                case 'a':  // Exclure ambigus
                    config->exclude_ambiguous = 1;
                    break;
            }
        }
        p++;
    }
}

// Fonction principale du générateur de mots de passe
void run_password_generator(const char* args) {
    password_generator_init();
    
    if (!args || *args == '\0') {
        // Génération par défaut
        password_config_t config;
        parse_password_args("", &config);
        
        char password[MAX_PASSWORD_LENGTH + 1];
        generate_password(password, &config);
        
        terminal_writestring("Mot de passe genere: ");
        terminal_writestring(password);
        terminal_writestring("\n");
        
        show_password_strength(password);
        return;
    }
    
    // Vérifier les commandes spéciales
    if (strcmp(args, "help") == 0 || strcmp(args, "-h") == 0) {
        show_password_help();
        return;
    }
    
    // Analyser un mot de passe existant
    if (strncmp(args, "-c ", 3) == 0) {
        const char* password_to_analyze = args + 3;
        while (*password_to_analyze == ' ') password_to_analyze++;
        
        terminal_writestring("Mot de passe: ");
        terminal_writestring(password_to_analyze);
        terminal_writestring("\n");
        show_password_strength(password_to_analyze);
        return;
    }
    
    // Générer plusieurs mots de passe
    if (strncmp(args, "-n ", 3) == 0) {
        const char* count_str = args + 3;
        while (*count_str == ' ') count_str++;
        
        int count = simple_atoi(count_str);
        password_config_t config;
        parse_password_args(args, &config);
        
        generate_multiple_passwords(count, &config);
        return;
    }
    
    // Génération normale avec options
    password_config_t config;
    parse_password_args(args, &config);
    
    char password[MAX_PASSWORD_LENGTH + 1];
    generate_password(password, &config);
    
    terminal_writestring("Mot de passe genere: ");
    terminal_writestring(password);
    terminal_writestring("\n");
    
    show_password_strength(password);
}
