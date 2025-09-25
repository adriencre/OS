#include "messaging.h"
#include "common.h"

// Variables globales
static user_t users[MAX_USERS];
static message_t messages[MAX_MESSAGES];
static int user_count = 0;
static int message_count = 0;
static char current_user[MAX_USERNAME_LEN] = "";
static int last_displayed_message = 0; // Pour tracking des nouveaux messages
static unsigned int last_auto_message_time = 0; // Timestamp du dernier message automatique
static int auto_message_index = 0; // Index du prochain message automatique
static int is_in_chat_ui = 0; // Flag pour savoir si on est dans l'interface chat

// Déclarations externes depuis kernel.c
extern void terminal_writestring(const char* data);
extern void clear_screen(void);
extern char getkey(void);
extern int strcmp(const char* s1, const char* s2);
extern void simple_itoa(int value, char* str);
extern unsigned int tick;

// Fonctions utilitaires externes (depuis blackjack.c)
extern void print_at(int x, int y, const char* text);
extern void show_message(const char* message);

// Fonction de hachage SHA-256 simplifiée pour l'OS
void simple_hash(const char* input, char* output) {
    // Implémentation d'un hachage plus robuste basé sur SHA-256 simplifié
    unsigned int h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    
    unsigned int len = 0;
    const char* str = input;
    while (*str++) len++; // Calculer la longueur
    
    // Traitement simplifié du message
    for (unsigned int i = 0; i < len; i++) {
        unsigned char c = input[i];
        h[i % 8] ^= c;
        h[i % 8] = (h[i % 8] << 7) | (h[i % 8] >> 25); // Rotation
        h[i % 8] += 0x9e3779b9; // Constante magique
    }
    
    // Mélange final
    for (int round = 0; round < 3; round++) {
        for (int i = 0; i < 8; i++) {
            h[i] ^= h[(i + 1) % 8];
            h[i] = (h[i] << 11) | (h[i] >> 21);
        }
    }
    
    // Convertir en hexadécimal (64 caractères)
    char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            output[i * 8 + j] = hex_chars[(h[i] >> (28 - j * 4)) & 0xF];
        }
    }
    output[64] = '\0';
}

// Copie de chaîne simple
void msg_strcpy(char* dest, const char* src, int max_len) {
    int i = 0;
    while (i < max_len - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Saisie sécurisée de mot de passe (affiche des *)
void secure_password_input(char* password, int max_len) {
    int pos = 0;
    char key;
    
    while (pos < max_len - 1) {
        key = getkey();
        
        if (key == '\n' || key == '\r') {
            break; // Entrée appuyée
        } else if (key == '\b' || key == 127) { // Backspace
            if (pos > 0) {
                pos--;
                terminal_writestring("\b \b"); // Effacer le caractère précédent
            }
        } else if (key >= 32 && key <= 126) { // Caractères imprimables
            password[pos] = key;
            pos++;
            terminal_writestring("*"); // Afficher une étoile
        }
    }
    
    password[pos] = '\0';
    terminal_writestring("\n");
}

// Saisie normale de texte
void secure_text_input(char* text, int max_len) {
    int pos = 0;
    char key;
    
    while (pos < max_len - 1) {
        key = getkey();
        
        if (key == '\n' || key == '\r') {
            break; // Entrée appuyée
        } else if (key == '\b' || key == 127) { // Backspace
            if (pos > 0) {
                pos--;
                terminal_writestring("\b \b"); // Effacer le caractère précédent
            }
        } else if (key >= 32 && key <= 126) { // Caractères imprimables
            text[pos] = key;
            pos++;
            // Afficher le caractère pour le nom d'utilisateur
            char display[2] = {key, '\0'};
            terminal_writestring(display);
        }
    }
    
    text[pos] = '\0';
    terminal_writestring("\n");
}

// Initialisation du système de messagerie
void messaging_init(void) {
    user_count = 0;
    message_count = 0;
    current_user[0] = '\0';
    
    // Ajout d'un compte administrateur par défaut
    messaging_register_user("admin", "nova2024");
    
    terminal_writestring("Sistema de messagerie NOVA initialise.\n");
    terminal_writestring("Compte administrateur cree: admin / nova2024\n");
}

// Trouver un utilisateur par nom
int find_user(const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

// Enregistrer un nouvel utilisateur
int messaging_register_user(const char* username, const char* password) {
    if (user_count >= MAX_USERS) {
        return 0; // Trop d'utilisateurs
    }
    
    if (find_user(username) != -1) {
        return 0; // Utilisateur existe déjà
    }
    
    msg_strcpy(users[user_count].username, username, MAX_USERNAME_LEN);
    simple_hash(password, users[user_count].password_hash);
    users[user_count].is_online = 0;
    user_count++;
    
    return 1; // Succès
}

// Connecter un utilisateur
int messaging_login_user(const char* username, const char* password) {
    int user_idx = find_user(username);
    if (user_idx == -1) {
        return 0; // Utilisateur non trouvé
    }
    
    char password_hash[65];
    simple_hash(password, password_hash);
    
    if (strcmp(users[user_idx].password_hash, password_hash) == 0) {
        users[user_idx].is_online = 1;
        msg_strcpy(current_user, username, MAX_USERNAME_LEN);
        return 1; // Succès
    }
    
    return 0; // Mot de passe incorrect
}

// Déconnecter un utilisateur
void messaging_logout_user(const char* username) {
    int user_idx = find_user(username);
    if (user_idx != -1) {
        users[user_idx].is_online = 0;
    }
    current_user[0] = '\0';
}

// Envoyer un message privé
int messaging_send_message(const char* sender, const char* recipient, const char* content) {
    if (message_count >= MAX_MESSAGES) {
        return 0; // Trop de messages
    }
    
    if (find_user(recipient) == -1) {
        return 0; // Destinataire introuvable
    }
    
    msg_strcpy(messages[message_count].sender, sender, MAX_USERNAME_LEN);
    msg_strcpy(messages[message_count].recipient, recipient, MAX_USERNAME_LEN);
    msg_strcpy(messages[message_count].content, content, MAX_MESSAGE_LEN);
    messages[message_count].timestamp = tick;
    messages[message_count].is_private = 1;
    message_count++;
    
    return 1; // Succès
}

// Envoyer un message global
int messaging_send_global_message(const char* sender, const char* content) {
    if (message_count >= MAX_MESSAGES) {
        return 0; // Trop de messages
    }
    
    msg_strcpy(messages[message_count].sender, sender, MAX_USERNAME_LEN);
    messages[message_count].recipient[0] = '\0'; // Message global
    msg_strcpy(messages[message_count].content, content, MAX_MESSAGE_LEN);
    messages[message_count].timestamp = tick;
    messages[message_count].is_private = 0;
    message_count++;
    
    return 1; // Succès
}

// Afficher les messages pour un utilisateur
void messaging_show_messages(const char* username) {
    terminal_writestring("=== VOS MESSAGES ===\n");
    
    int found = 0;
    for (int i = 0; i < message_count; i++) {
        // Afficher messages globaux et messages privés pour cet utilisateur
        if (!messages[i].is_private || 
            strcmp(messages[i].recipient, username) == 0 ||
            strcmp(messages[i].sender, username) == 0) {
            
            terminal_writestring("[");
            if (messages[i].is_private) {
                terminal_writestring("PRIVE] ");
            } else {
                terminal_writestring("PUBLIC] ");
            }
            
            terminal_writestring(messages[i].sender);
            terminal_writestring(": ");
            terminal_writestring(messages[i].content);
            terminal_writestring("\n");
            found = 1;
        }
    }
    
    if (!found) {
        terminal_writestring("Aucun message.\n");
    }
    terminal_writestring("\n");
}

// Afficher les utilisateurs en ligne
void messaging_show_users_online(void) {
    terminal_writestring("=== UTILISATEURS EN LIGNE ===\n");
    
    int found = 0;
    for (int i = 0; i < user_count; i++) {
        if (users[i].is_online) {
            terminal_writestring("- ");
            terminal_writestring(users[i].username);
            terminal_writestring("\n");
            found = 1;
        }
    }
    
    if (!found) {
        terminal_writestring("Aucun utilisateur en ligne.\n");
    }
    terminal_writestring("\n");
}

// Interface utilisateur du système de messagerie
void messaging_ui(void) {
    // Désactiver les messages automatiques pendant qu'on est dans l'interface chat
    messaging_set_ui_mode(1);
    
    clear_screen();
    
    terminal_writestring("=== SYSTEME DE MESSAGERIE NOVA ===\n\n");
    terminal_writestring("Bienvenue dans le chat NOVA!\n\n");
    
    if (current_user[0] == '\0') {
        // Pas connecté - menu de connexion
        terminal_writestring("MENU PRINCIPAL:\n");
        terminal_writestring("1. Se connecter\n");
        terminal_writestring("2. S'inscrire\n"); 
        terminal_writestring("3. Utilisateurs en ligne\n");
        terminal_writestring("4. Quitter\n\n");
        terminal_writestring("Appuyez sur 1, 2, 3 ou 4: ");
        
        char key = getkey();
        
        if (key == '1') {
            // Connexion avec saisie sécurisée
            clear_screen();
            terminal_writestring("=== CONNEXION SECURISEE ===\n\n");
            
            char username[MAX_USERNAME_LEN];
            char password[MAX_PASSWORD_LEN];
            
            terminal_writestring("Nom d'utilisateur: ");
            secure_text_input(username, MAX_USERNAME_LEN);
            
            terminal_writestring("Mot de passe: ");
            secure_password_input(password, MAX_PASSWORD_LEN);
            
            terminal_writestring("\nVerification en cours...\n");
            
            if (messaging_login_user(username, password)) {
                terminal_writestring("Connexion reussie! Bienvenue ");
                terminal_writestring(username);
                terminal_writestring("!\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
            } else {
                terminal_writestring("Echec de la connexion! Nom d'utilisateur ou mot de passe incorrect.\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
            }
        }
        else if (key == '2') {
            // Inscription avec saisie sécurisée
            clear_screen();
            terminal_writestring("=== INSCRIPTION SECURISEE ===\n\n");
            
            char username[MAX_USERNAME_LEN];
            char password[MAX_PASSWORD_LEN];
            char confirm_password[MAX_PASSWORD_LEN];
            
            terminal_writestring("Nom d'utilisateur: ");
            secure_text_input(username, MAX_USERNAME_LEN);
            
            terminal_writestring("Mot de passe: ");
            secure_password_input(password, MAX_PASSWORD_LEN);
            
            terminal_writestring("Confirmez le mot de passe: ");
            secure_password_input(confirm_password, MAX_PASSWORD_LEN);
            
            // Vérifier que les mots de passe correspondent
            if (strcmp(password, confirm_password) != 0) {
                terminal_writestring("\nErreur: Les mots de passe ne correspondent pas!\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
                return;
            }
            
            // Vérifier que le nom d'utilisateur n'est pas vide
            if (username[0] == '\0') {
                terminal_writestring("\nErreur: Le nom d'utilisateur ne peut pas etre vide!\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
                return;
            }
            
            terminal_writestring("\nCreation du compte...\n");
            
            if (messaging_register_user(username, password)) {
                terminal_writestring("Inscription reussie! Compte cree pour ");
                terminal_writestring(username);
                terminal_writestring("!\n");
                terminal_writestring("Vous pouvez maintenant vous connecter.\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
            } else {
                terminal_writestring("Echec de l'inscription! Nom d'utilisateur deja pris ou limite atteinte.\n");
                terminal_writestring("Appuyez sur une touche pour continuer...");
                getkey();
            }
        }
        else if (key == '3') {
            terminal_writestring("3\n");
            clear_screen();
            messaging_show_users_online();
            terminal_writestring("Appuyez sur une touche pour continuer...");
            getkey();
        }
        else if (key == '4') {
            terminal_writestring("4\n");
            terminal_writestring("Au revoir!\n");
            messaging_set_ui_mode(0); // Réactiver les messages automatiques
            return;
        }
        else {
            terminal_writestring("\nChoix invalide! Appuyez sur 1, 2, 3 ou 4.\n");
            terminal_writestring("Appuyez sur une touche pour continuer...");
            getkey();
        }
    } else {
        // Connecté - menu principal
        terminal_writestring("Connecte en tant que: ");
        terminal_writestring(current_user);
        terminal_writestring("\n\n");
        
        terminal_writestring("MENU UTILISATEUR:\n");
        terminal_writestring("1. Voir mes messages\n");
        terminal_writestring("2. Envoyer message global\n");
        terminal_writestring("3. Envoyer message prive\n");
        terminal_writestring("4. Utilisateurs en ligne\n");
        terminal_writestring("5. Se deconnecter\n");
        terminal_writestring("6. Quitter\n\n");
        terminal_writestring("Choisissez une option (1-6): ");
        
        char key = getkey();
        
        if (key == '1') {
            terminal_writestring("1\n");
            clear_screen();
            messaging_show_messages(current_user);
            terminal_writestring("Appuyez sur une touche pour continuer...");
            getkey();
        }
        else if (key == '2') {
            terminal_writestring("2\n");
            clear_screen();
            terminal_writestring("=== MESSAGE GLOBAL ===\n\n");
            terminal_writestring("Messages predefinnis:\n");
            terminal_writestring("H - Hello tout le monde!\n");
            terminal_writestring("N - NOVA est genial!\n");
            terminal_writestring("U - Ultron arrive...\n\n");
            terminal_writestring("Choisissez H, N ou U: ");
            
            char msg_key = getkey();
            char* message = "";
            if (msg_key == 'h' || msg_key == 'H') {
                message = "Hello tout le monde!";
                terminal_writestring("H\n");
            } else if (msg_key == 'n' || msg_key == 'N') {
                message = "NOVA est genial!";
                terminal_writestring("N\n");
            } else if (msg_key == 'u' || msg_key == 'U') {
                message = "Ultron arrive...";
                terminal_writestring("U\n");
            } else {
                terminal_writestring("\nChoix invalide!\n");
            }
            
            if (message[0] != '\0') {
                if (messaging_send_global_message(current_user, message)) {
                    terminal_writestring("Message envoye!\n");
                } else {
                    terminal_writestring("Erreur envoi message!\n");
                }
            }
            terminal_writestring("Appuyez sur une touche pour continuer...");
            getkey();
        }
        else if (key == '3') {
            terminal_writestring("3\n");
            terminal_writestring("Messages prives: Fonctionnalite en developpement\n");
            terminal_writestring("Appuyez sur une touche pour continuer...");
            getkey();
        }
        else if (key == '4') {
            terminal_writestring("4\n");
            clear_screen();
            messaging_show_users_online();
            terminal_writestring("Appuyez sur une touche pour continuer...");
            getkey();
        }
        else if (key == '5') {
            terminal_writestring("5\n");
            messaging_logout_user(current_user);
            terminal_writestring("Deconnexion reussie!\n");
            terminal_writestring("Appuyez sur une touche pour continuer...");
            getkey();
        }
        else if (key == '6') {
            terminal_writestring("6\n");
            messaging_logout_user(current_user);
            terminal_writestring("Au revoir!\n");
            messaging_set_ui_mode(0); // Réactiver les messages automatiques
            return;
        }
        else {
            terminal_writestring("\nChoix invalide! Appuyez sur 1, 2, 3, 4, 5 ou 6.\n");
            terminal_writestring("Appuyez sur une touche pour continuer...");
            getkey();
        }
    }
    
    // Boucle continue (récursive)
    messaging_ui();
}

// Point d'entrée principal pour la commande chat
void do_messaging(void) {
    messaging_ui();
    
    // Réactiver les messages automatiques en quittant l'interface chat
    messaging_set_ui_mode(0);
    
    clear_screen();
    terminal_writestring("Retour au shell NOVA.\n");
}

// Vérifier et afficher les nouveaux messages globaux
void messaging_check_new_messages(void) {
    for (int i = last_displayed_message; i < message_count; i++) {
        // Afficher seulement les messages globaux (non privés)
        if (!messages[i].is_private) {
            terminal_writestring("\r"); // Retour chariot pour effacer le prompt
            terminal_writestring("[MESSAGE GLOBAL] ");
            terminal_writestring(messages[i].sender);
            terminal_writestring(": ");
            terminal_writestring(messages[i].content);
            terminal_writestring("\n");
        }
    }
    last_displayed_message = message_count;
}

// Vérifier s'il y a de nouveaux messages
int messaging_has_new_messages(void) {
    for (int i = last_displayed_message; i < message_count; i++) {
        if (!messages[i].is_private) {
            return 1; // Il y a au moins un nouveau message global
        }
    }
    return 0;
}

// Afficher le prompt avec notifications
void messaging_display_prompt_with_notifications(void) {
    if (messaging_has_new_messages()) {
        terminal_writestring("[MSG] > ");
    } else {
        terminal_writestring("> ");
    }
}

// Obtenir l'utilisateur actuel
const char* messaging_get_current_user(void) {
    if (current_user[0] != '\0') {
        return current_user;
    }
    return "guest"; // Utilisateur par défaut si pas connecté
}

// Vérifier et envoyer des messages automatiques
void messaging_check_auto_messages(void) {
    // Ne pas afficher les messages automatiques si on est dans l'interface chat
    if (is_in_chat_ui) {
        return;
    }
    
    // Messages automatiques prédéfinis
    static const char* auto_messages[] = {
        "Système NOVA opérationnel - Tout va bien!",
        "Attention: Ultron pourrait attaquer bientôt...",
        "Rappel: Sauvegardez vos données importantes",
        "Info: Nouveau module Wikipedia disponible",
        "Alerte: Vérifiez votre utilisation mémoire",
        "Message du système: NOVA v2.0 fonctionne parfaitement",
        "Notification: Chat inter-machines en développement",
        "Status: Tous les systèmes sont opérationnels"
    };
    
    static const int num_auto_messages = sizeof(auto_messages) / sizeof(auto_messages[0]);
    
    // Vérifier si c'est le moment d'envoyer un message automatique
    // Timer = 100Hz, donc 1000 ticks = 10 secondes
    if (tick >= last_auto_message_time + 1000) {
        // Envoyer le message automatique
        const char* message = auto_messages[auto_message_index];
        
        if (messaging_send_global_message("SYSTEME", message)) {
            // Afficher immédiatement le message (sans attendre la prochaine commande)
            terminal_writestring("\r\n[MESSAGE GLOBAL] SYSTEME: ");
            terminal_writestring(message);
            terminal_writestring("\n");
            
            // Marquer comme affiché pour éviter la double affichage
            last_displayed_message = message_count;
            
            // Préparer le prochain message
            auto_message_index = (auto_message_index + 1) % num_auto_messages;
            last_auto_message_time = tick;
        }
    }
}

// Définir si on est dans l'interface chat ou pas
void messaging_set_ui_mode(int in_chat_ui) {
    is_in_chat_ui = in_chat_ui;
}
