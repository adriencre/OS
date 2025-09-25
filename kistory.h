#ifndef KISTORY_H
#define KISTORY_H

#include "common.h"

#define MAX_HISTORY_SIZE 20
#define MAX_COMMAND_LENGTH 128

typedef struct {
    char command[MAX_COMMAND_LENGTH];
    uint32_t timestamp;
    int is_favorite;
} history_entry_t;

extern history_entry_t history_commands[MAX_HISTORY_SIZE];
extern int history_count;
extern int history_index;
extern int history_navigation_index;

void kistory_init();
void kistory_add_command(const char* command);
const char* kistory_get_previous();
const char* kistory_get_next();
void kistory_reset_navigation();
void kistory_display_all();
void kistory_clear();
void kistory_search(const char* query);
void kistory_toggle_favorite(int index);
void kistory_display_favorites();
void kistory_system();

#endif
