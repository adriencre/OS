#ifndef MESSAGING_H
#define MESSAGING_H

#define MAX_USERS 10
#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 32
#define MAX_MESSAGE_LEN 256
#define MAX_MESSAGES 50
#define AUTO_MESSAGE_INTERVAL 10   // 10 secondes (10 ticks d'environ 1 seconde)

// Structure pour un utilisateur
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password_hash[65]; // SHA-256 (simplifié)
    int is_online;
} user_t;

// Structure pour un message
typedef struct {
    char sender[MAX_USERNAME_LEN];
    char recipient[MAX_USERNAME_LEN]; // "" pour message global
    char content[MAX_MESSAGE_LEN];
    unsigned int timestamp;
    int is_private;
} message_t;

// Fonctions principales
void messaging_init(void);
int messaging_register_user(const char* username, const char* password);
int messaging_login_user(const char* username, const char* password);
void messaging_logout_user(const char* username);
int messaging_send_message(const char* sender, const char* recipient, const char* content);
int messaging_send_global_message(const char* sender, const char* content);
void messaging_show_messages(const char* username);
void messaging_show_users_online(void);
void messaging_ui(void);

// Nouvelles fonctions pour intégration shell
void messaging_check_new_messages(void);
int messaging_has_new_messages(void);
void messaging_display_prompt_with_notifications(void);
const char* messaging_get_current_user(void);
void messaging_check_auto_messages(void);
void messaging_set_ui_mode(int in_chat_ui);

// Fonctions utilitaires
void simple_hash(const char* input, char* output);
int find_user(const char* username);

#endif
