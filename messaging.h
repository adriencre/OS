#ifndef MESSAGING_H
#define MESSAGING_H

#include "common.h"

// Constantes du système de messagerie
#define MAX_USERS 10
#define MAX_MESSAGES 50
#define MAX_USERNAME_LEN 16
#define MAX_MESSAGE_LEN 128
#define MAX_PASSWORD_LEN 32

// Types de messages
typedef enum {
    MSG_GLOBAL = 0,
    MSG_PRIVATE = 1,
    MSG_SYSTEM = 2
} message_type_t;

// Structure utilisateur
typedef struct {
    char username[MAX_USERNAME_LEN];
    uint32_t password_hash;
    int is_online;
    uint32_t last_activity;
} user_t;

// Structure message
typedef struct {
    char sender[MAX_USERNAME_LEN];
    char recipient[MAX_USERNAME_LEN];
    char content[MAX_MESSAGE_LEN];
    uint32_t timestamp;
    message_type_t type;
    int is_read;
} message_t;

// Variables globales du système
extern user_t users[MAX_USERS];
extern message_t messages[MAX_MESSAGES];
extern int user_count;
extern int message_count;
extern char current_user[MAX_USERNAME_LEN];
extern int current_user_id;
extern int has_new_messages;

// Fonctions principales
void messaging_init();
void messaging_system();
int authenticate_user(const char* username, const char* password);
int register_user(const char* username, const char* password);
void send_message(const char* recipient, const char* content, message_type_t type);
void display_messages();
void check_new_messages();
uint32_t simple_hash(const char* str);
void get_password_input(char* buffer, size_t max_len);
void messaging_menu();
void login_menu();
void register_menu();
void message_compose_menu();
void message_list_menu();

#endif