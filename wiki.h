#ifndef WIKI_H
#define WIKI_H

#include "common.h"

// Configuration de la mini-Wikipedia
#define MAX_ARTICLES 100
#define MAX_TITLE_LEN 64
#define MAX_CONTENT_LEN 512
#define MAX_KEYWORDS_LEN 64

// Structure d'un article
typedef struct {
    char title[MAX_TITLE_LEN];
    char content[MAX_CONTENT_LEN];
    char keywords[MAX_KEYWORDS_LEN];
} wiki_article_t;

// Fonctions principales
void wiki_init(void);
int wiki_search(const char* query, int results[], int max_results);
int wiki_get_article_count(void);

// Fonctions pour l'interface utilisateur
void do_wiki(char* args);
void wiki_interactive_ui(void);

#endif // WIKI_H
