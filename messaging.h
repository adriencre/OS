#ifndef MESSAGING_H
#define MESSAGING_H

#include "common.h"

// Constantes pour le système de messagerie
#define MAX_USERS 50
#define MAX_MESSAGES 200
#define MAX_USERNAME_LEN 32
#define MAX_MESSAGE_LEN 256
#define MAX_PASSWORD_LEN 64

// Structure pour un utilisateur
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password_hash[65]; // SHA-256 en hex = 64 caractères + \0
    int is_online;
    unsigned int last_activity;
} user_t;

// Structure pour un message
typedef struct {
    char sender[MAX_USERNAME_LEN];
    char recipient[MAX_USERNAME_LEN]; // "ALL" pour message global
    char content[MAX_MESSAGE_LEN];
    unsigned int timestamp;
    int is_read;
    int is_private; // 1 pour message privé, 0 pour message global
} message_t;

// Déclarations des fonctions publiques
void messaging_init(void);
void do_messaging(void);
void messaging_display_prompt_with_notifications(void);
void messaging_check_auto_messages(void);
int messaging_register_user(const char* username, const char* password);
void messaging_set_ui_mode(int in_chat_ui);

#endif
