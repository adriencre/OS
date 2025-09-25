#include "messaging.h"
#include "timer.h"
#include "keyboard.h"

// Déclarations externes nécessaires
extern void terminal_writestring(const char* data);
extern void terminal_putchar(char c);
extern void clear_screen();
extern uint8_t terminal_color;
extern uint32_t tick;
extern int strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);
extern size_t strlen(const char* s);

// Variables globales du système de messagerie
user_t users[MAX_USERS];
message_t messages[MAX_MESSAGES];
int user_count = 0;
int message_count = 0;
char current_user[MAX_USERNAME_LEN] = "";
int current_user_id = -1;
int has_new_messages = 0;

// Fonction de hachage simple (SHA-256 simplifié)
uint32_t simple_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Saisie de mot de passe masquée
void get_password_input(char* buffer, size_t max_len) {
    size_t len = 0;
    terminal_writestring("Mot de passe: ");
    
    while (len < max_len - 1) {
        int key = getkey();
        if (key == '\n') {
            break;
        } else if (key == '\b') {
            if (len > 0) {
                len--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        } else if (key > 0 && key < 256) {
            buffer[len] = (char)key;
            len++;
            terminal_putchar('*'); // Masquer le caractère
        }
    }
    buffer[len] = '\0';
    terminal_putchar('\n');
}

// Initialisation du système de messagerie
void messaging_init() {
    user_count = 0;
    message_count = 0;
    current_user_id = -1;
    has_new_messages = 0;
    
    // Créer un utilisateur admin par défaut
    strcpy(users[0].username, "admin");
    users[0].password_hash = simple_hash("admin123");
    users[0].is_online = 0;
    users[0].last_activity = 0;
    user_count = 1;
    
    // Message de bienvenue système
    strcpy(messages[0].sender, "SYSTEM");
    strcpy(messages[0].recipient, "ALL");
    strcpy(messages[0].content, "Bienvenue dans le systeme de messagerie NOVA!");
    messages[0].timestamp = tick;
    messages[0].type = MSG_SYSTEM;
    messages[0].is_read = 0;
    message_count = 1;
}

// Authentification utilisateur
int authenticate_user(const char* username, const char* password) {
    uint32_t password_hash = simple_hash(password);
    
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            if (users[i].password_hash == password_hash) {
                current_user_id = i;
                strcpy(current_user, username);
                users[i].is_online = 1;
                users[i].last_activity = tick;
                return 1;
            }
        }
    }
    return 0;
}

// Enregistrement d'un nouvel utilisateur
int register_user(const char* username, const char* password) {
    if (user_count >= MAX_USERS) {
        return 0; // Trop d'utilisateurs
    }
    
    // Vérifier si l'utilisateur existe déjà
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return 0; // Utilisateur déjà existant
        }
    }
    
    // Créer le nouvel utilisateur
    strcpy(users[user_count].username, username);
    users[user_count].password_hash = simple_hash(password);
    users[user_count].is_online = 0;
    users[user_count].last_activity = 0;
    user_count++;
    
    return 1;
}

// Envoyer un message
void send_message(const char* recipient, const char* content, message_type_t type) {
    if (message_count >= MAX_MESSAGES) {
        // Décaler les messages anciens
        for (int i = 0; i < MAX_MESSAGES - 1; i++) {
            messages[i] = messages[i + 1];
        }
        message_count = MAX_MESSAGES - 1;
    }
    
    strcpy(messages[message_count].sender, current_user);
    strcpy(messages[message_count].recipient, recipient);
    strcpy(messages[message_count].content, content);
    messages[message_count].timestamp = tick;
    messages[message_count].type = type;
    messages[message_count].is_read = 0;
    message_count++;
    
    has_new_messages = 1;
}

// Afficher les messages
void display_messages() {
    terminal_writestring("=== MESSAGES ===\n");
    
    int displayed = 0;
    for (int i = message_count - 1; i >= 0 && displayed < 10; i--) {
        message_t* msg = &messages[i];
        
        // Vérifier si l'utilisateur peut voir ce message
        int can_see = 0;
        if (msg->type == MSG_GLOBAL || msg->type == MSG_SYSTEM) {
            can_see = 1;
        } else if (msg->type == MSG_PRIVATE) {
            can_see = (strcmp(msg->recipient, current_user) == 0 || 
                      strcmp(msg->sender, current_user) == 0);
        }
        
        if (can_see) {
            // Couleur selon le type
            uint8_t color = 0x0F; // Blanc par défaut
            if (msg->type == MSG_SYSTEM) color = 0x0C; // Rouge
            else if (msg->type == MSG_PRIVATE) color = 0x0A; // Vert
            else if (strcmp(msg->sender, current_user) == 0) color = 0x0B; // Cyan
            
            // Sauvegarder la couleur actuelle
            uint8_t old_color = terminal_color;
            terminal_color = color;
            
            terminal_writestring("[");
            terminal_writestring(msg->sender);
            terminal_writestring("] ");
            terminal_writestring(msg->content);
            terminal_writestring("\n");
            
            // Restaurer la couleur
            terminal_color = old_color;
            displayed++;
        }
    }
    
    if (displayed == 0) {
        terminal_writestring("Aucun message.\n");
    }
}

// Vérifier les nouveaux messages
void check_new_messages() {
    if (has_new_messages) {
        terminal_writestring("[MSG] ");
        has_new_messages = 0;
    }
}

// Menu principal de messagerie
void messaging_menu() {
    while (1) {
        clear_screen();
        terminal_writestring("=== SYSTEME DE MESSAGERIE NOVA ===\n");
        terminal_writestring("Utilisateur connecte: ");
        terminal_writestring(current_user);
        terminal_writestring("\n\n");
        
        terminal_writestring("1. Voir les messages\n");
        terminal_writestring("2. Envoyer un message global\n");
        terminal_writestring("3. Envoyer un message prive\n");
        terminal_writestring("4. Deconnexion\n");
        terminal_writestring("\nChoix: ");
        
        int choice = getkey() - '0';
        terminal_putchar('\n');
        
        switch (choice) {
            case 1:
                display_messages();
                terminal_writestring("\nAppuyez sur une touche pour continuer...");
                getkey();
                break;
            case 2:
                message_compose_menu();
                break;
            case 3:
                message_list_menu();
                break;
            case 4:
                if (current_user_id >= 0) {
                    users[current_user_id].is_online = 0;
                }
                current_user_id = -1;
                strcpy(current_user, "");
                return;
            default:
                terminal_writestring("Choix invalide.\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
        }
    }
}

// Menu de connexion
void login_menu() {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    
    terminal_writestring("=== CONNEXION ===\n");
    terminal_writestring("Nom d'utilisateur: ");
    
    // Lire le nom d'utilisateur
    size_t len = 0;
    while (len < MAX_USERNAME_LEN - 1) {
        int key = getkey();
        if (key == '\n') break;
        if (key == '\b' && len > 0) {
            len--;
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
        } else if (key > 0 && key < 256) {
            username[len] = (char)key;
            len++;
            terminal_putchar(key);
        }
    }
    username[len] = '\0';
    terminal_putchar('\n');
    
    get_password_input(password, MAX_PASSWORD_LEN);
    
    if (authenticate_user(username, password)) {
        terminal_writestring("Connexion reussie!\n");
        terminal_writestring("Appuyez sur une touche pour continuer...");
        getkey();
        messaging_menu();
    } else {
        terminal_writestring("Echec de la connexion.\n");
        terminal_writestring("Appuyez sur une touche pour continuer...");
        getkey();
    }
}

// Menu d'inscription
void register_menu() {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    
    terminal_writestring("=== INSCRIPTION ===\n");
    terminal_writestring("Nom d'utilisateur: ");
    
    // Lire le nom d'utilisateur
    size_t len = 0;
    while (len < MAX_USERNAME_LEN - 1) {
        int key = getkey();
        if (key == '\n') break;
        if (key == '\b' && len > 0) {
            len--;
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
        } else if (key > 0 && key < 256) {
            username[len] = (char)key;
            len++;
            terminal_putchar(key);
        }
    }
    username[len] = '\0';
    terminal_putchar('\n');
    
    get_password_input(password, MAX_PASSWORD_LEN);
    
    if (register_user(username, password)) {
        terminal_writestring("Inscription reussie!\n");
        terminal_writestring("Appuyez sur une touche pour continuer...");
        getkey();
    } else {
        terminal_writestring("Echec de l'inscription (utilisateur deja existant).\n");
        terminal_writestring("Appuyez sur une touche pour continuer...");
        getkey();
    }
}

// Menu de composition de message
void message_compose_menu() {
    char content[MAX_MESSAGE_LEN];
    char recipient[MAX_USERNAME_LEN];
    
    terminal_writestring("=== ENVOYER MESSAGE PRIVE ===\n");
    terminal_writestring("Destinataire: ");
    
    // Lire le destinataire
    size_t len = 0;
    while (len < MAX_USERNAME_LEN - 1) {
        int key = getkey();
        if (key == '\n') break;
        if (key == '\b' && len > 0) {
            len--;
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
        } else if (key > 0 && key < 256) {
            recipient[len] = (char)key;
            len++;
            terminal_putchar(key);
        }
    }
    recipient[len] = '\0';
    terminal_putchar('\n');
    
    terminal_writestring("Message: ");
    
    // Lire le message
    len = 0;
    while (len < MAX_MESSAGE_LEN - 1) {
        int key = getkey();
        if (key == '\n') break;
        if (key == '\b' && len > 0) {
            len--;
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
        } else if (key > 0 && key < 256) {
            content[len] = (char)key;
            len++;
            terminal_putchar(key);
        }
    }
    content[len] = '\0';
    terminal_putchar('\n');
    
    send_message(recipient, content, MSG_PRIVATE);
    terminal_writestring("Message envoye!\n");
    terminal_writestring("Appuyez sur une touche pour continuer...");
    getkey();
}

// Menu de liste des messages
void message_list_menu() {
    char content[MAX_MESSAGE_LEN];
    
    terminal_writestring("=== MESSAGE GLOBAL ===\n");
    terminal_writestring("Message: ");
    
    // Lire le message
    size_t len = 0;
    while (len < MAX_MESSAGE_LEN - 1) {
        int key = getkey();
        if (key == '\n') break;
        if (key == '\b' && len > 0) {
            len--;
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
        } else if (key > 0 && key < 256) {
            content[len] = (char)key;
            len++;
            terminal_putchar(key);
        }
    }
    content[len] = '\0';
    terminal_putchar('\n');
    
    send_message("ALL", content, MSG_GLOBAL);
    terminal_writestring("Message global envoye!\n");
    terminal_writestring("Appuyez sur une touche pour continuer...");
    getkey();
}

// Système principal de messagerie
void messaging_system() {
    messaging_init();
    
    while (1) {
        clear_screen();
        terminal_writestring("=== SYSTEME DE MESSAGERIE NOVA ===\n\n");
        terminal_writestring("1. Connexion\n");
        terminal_writestring("2. Inscription\n");
        terminal_writestring("3. Retour au shell\n");
        terminal_writestring("\nChoix: ");
        
        int choice = getkey() - '0';
        terminal_putchar('\n');
        
        switch (choice) {
            case 1:
                login_menu();
                break;
            case 2:
                register_menu();
                break;
            case 3:
                return;
            default:
                terminal_writestring("Choix invalide.\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
        }
    }
}