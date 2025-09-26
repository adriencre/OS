#include "wiki.h"
#include "wiki_data.h"

// Variables globales pour stocker les articles
static wiki_article_t wiki_articles[MAX_ARTICLES];
static int wiki_article_count = 0;

// --- Fonctions externes du kernel ---
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void clear_screen(void);
int getkey();
char* simple_itoa(int num, char* buffer, int base);
int strcmp(const char* s1, const char* s2);

// --- Fonctions d'affichage du blackjack (déclarations externes) ---
void terminal_putentryat(char c, uint8_t color, int x, int y);
void print_at(const char* str, uint8_t color, int x, int y);  // Définie dans blackjack.c
void show_message(const char* msg, uint8_t color);           // Définie dans blackjack.c

// Couleurs VGA (comme dans blackjack)
#define COLOR_BLACK          0
#define COLOR_BLUE           1
#define COLOR_GREEN          2
#define COLOR_CYAN           3
#define COLOR_RED            4
#define COLOR_MAGENTA        5
#define COLOR_BROWN          6
#define COLOR_LIGHT_GREY     7
#define COLOR_DARK_GREY      8
#define COLOR_LIGHT_BLUE     9
#define COLOR_LIGHT_GREEN    10
#define COLOR_LIGHT_CYAN     11
#define COLOR_LIGHT_RED      12
#define COLOR_LIGHT_MAGENTA  13
#define COLOR_YELLOW         14
#define COLOR_WHITE          15

#define VGA_COLOR(fg, bg) (fg | bg << 4)

// Fonctions utilitaires internes
static void wiki_strcpy(char* dest, const char* src, int max_len) {
    int i = 0;
    while (i < max_len - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static char wiki_tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

static int wiki_contains(const char* text, const char* search) {
    int text_len = 0, search_len = 0;
    
    // Calculer les longueurs
    while (text[text_len] != '\0') text_len++;
    while (search[search_len] != '\0') search_len++;
    
    if (search_len == 0) return 1;
    if (text_len < search_len) return 0;
    
    // Recherche insensible à la casse
    for (int i = 0; i <= text_len - search_len; i++) {
        int match = 1;
        for (int j = 0; j < search_len; j++) {
            if (wiki_tolower(text[i + j]) != wiki_tolower(search[j])) {
                match = 0;
                break;
            }
        }
        if (match) return 1;
    }
    return 0;
}

// Parser une ligne de données (format: TITRE|MOTS_CLES|CONTENU)
void parse_article_line(const char* line) {
    if (wiki_article_count >= MAX_ARTICLES) return;
    
    char title[MAX_TITLE_LEN];
    char keywords[MAX_KEYWORDS_LEN];
    char content[MAX_CONTENT_LEN];
    
    int state = 0; // 0=titre, 1=mots-clés, 2=contenu
    int pos = 0;
    int line_pos = 0;
    
    // Parser la ligne
    while (line[line_pos] != '\0') {
        char c = line[line_pos];
        
        if (c == '|') {
            // Fin d'une section
            if (state == 0) {
                title[pos] = '\0';
                state = 1;
                pos = 0;
            } else if (state == 1) {
                keywords[pos] = '\0';
                state = 2;
                pos = 0;
            }
        } else {
            // Ajouter le caractère à la section actuelle
            if (state == 0 && pos < MAX_TITLE_LEN - 1) {
                title[pos++] = c;
            } else if (state == 1 && pos < MAX_KEYWORDS_LEN - 1) {
                keywords[pos++] = c;
            } else if (state == 2 && pos < MAX_CONTENT_LEN - 1) {
                content[pos++] = c;
            }
        }
        line_pos++;
    }
    
    // Finir la dernière section (contenu)
    if (state == 2) {
        content[pos] = '\0';
        
        // Ajouter l'article
        wiki_strcpy(wiki_articles[wiki_article_count].title, title, MAX_TITLE_LEN);
        wiki_strcpy(wiki_articles[wiki_article_count].keywords, keywords, MAX_KEYWORDS_LEN);
        wiki_strcpy(wiki_articles[wiki_article_count].content, content, MAX_CONTENT_LEN);
        wiki_article_count++;
    }
}

// Initialiser la mini-Wikipedia avec les données externes
void wiki_init(void) {
    wiki_article_count = 0;
    
    // Charger tous les articles depuis wiki_data.h
    for (int i = 0; wiki_articles_data[i] != NULL; i++) {
        parse_article_line(wiki_articles_data[i]);
    }
    
    terminal_writestring("Mini-Wikipedia initialisee avec ");
    char buffer[16];
    simple_itoa(wiki_article_count, buffer, 10);
    terminal_writestring(buffer);
    terminal_writestring(" articles.\n");
}

// Ajouter un nouvel article
void wiki_add_article(const char* title, const char* content, const char* keywords) {
    if (wiki_article_count >= MAX_ARTICLES) return;
    
    wiki_strcpy(wiki_articles[wiki_article_count].title, title, MAX_TITLE_LEN);
    wiki_strcpy(wiki_articles[wiki_article_count].content, content, MAX_CONTENT_LEN);
    wiki_strcpy(wiki_articles[wiki_article_count].keywords, keywords, MAX_KEYWORDS_LEN);
    wiki_article_count++;
}

// Rechercher des articles
int wiki_search(const char* query, int results[], int max_results) {
    int found = 0;
    
    for (int i = 0; i < wiki_article_count && found < max_results; i++) {
        // Chercher dans le titre, contenu et mots-clés
        if (wiki_contains(wiki_articles[i].title, query) ||
            wiki_contains(wiki_articles[i].content, query) ||
            wiki_contains(wiki_articles[i].keywords, query)) {
            results[found] = i;
            found++;
        }
    }
    
    return found;
}


// Obtenir le nombre d'articles
int wiki_get_article_count(void) {
    return wiki_article_count;
}

// Interface graphique simplifiée (style blackjack)
void wiki_draw_screen() {
    clear_screen();
    
    // Titre en haut (style blackjack)
    print_at("=================== MINI WIKIPEDIA ===================", 
             VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 12, 1);
    
    // Bordures
    for (int y = 3; y < 22; y++) {
        terminal_putentryat('|', VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 1, y);
        terminal_putentryat('|', VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 78, y);
    }
    
    // Ligne du bas pour instructions
    for (int x = 0; x < 80; x++) {
        terminal_putentryat('-', VGA_COLOR(COLOR_CYAN, COLOR_BLACK), x, 22);
    }
}

void wiki_show_menu() {
    wiki_draw_screen();
    
    // Menu principal
    print_at("Menu Principal:", VGA_COLOR(COLOR_WHITE, COLOR_BLACK), 5, 5);
    print_at("1. Liste des articles", VGA_COLOR(COLOR_LIGHT_GREEN, COLOR_BLACK), 7, 7);
    print_at("2. Rechercher", VGA_COLOR(COLOR_LIGHT_GREEN, COLOR_BLACK), 7, 8);
    print_at("3. Article aleatoire", VGA_COLOR(COLOR_LIGHT_GREEN, COLOR_BLACK), 7, 9);
    print_at("Q. Quitter", VGA_COLOR(COLOR_LIGHT_RED, COLOR_BLACK), 7, 11);
    
    show_message("Choisissez une option (1-3, Q)", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
}


// Interface principale simplifiée (style blackjack)
void wiki_interactive_ui() {
    while (1) {
        wiki_show_menu();
        int choice = getkey();
        
        switch (choice) {
            case '1': {
                // Liste des articles avec pagination
                int articles_per_page = 12;
                int current_page = 0;
                int total_pages = (wiki_article_count + articles_per_page - 1) / articles_per_page;
                
                while (1) {
                    wiki_draw_screen();
                    
                    // Titre avec info de page
                    char page_info[64];
                    char page_str[8], total_str[8];
                    simple_itoa(current_page + 1, page_str, 10);
                    simple_itoa(total_pages, total_str, 10);
                    
                    int pos = 0;
                    char* prefix = "Articles (page ";
                    for (int i = 0; prefix[i] != '\0'; i++) page_info[pos++] = prefix[i];
                    for (int i = 0; page_str[i] != '\0'; i++) page_info[pos++] = page_str[i];
                    page_info[pos++] = '/';
                    for (int i = 0; total_str[i] != '\0'; i++) page_info[pos++] = total_str[i];
                    page_info[pos++] = ')';
                    page_info[pos++] = ':';
                    page_info[pos] = '\0';
                    
                    print_at(page_info, VGA_COLOR(COLOR_WHITE, COLOR_BLACK), 5, 5);
                    
                    // Afficher les articles de la page actuelle
                    int start_idx = current_page * articles_per_page;
                    int end_idx = start_idx + articles_per_page;
                    if (end_idx > wiki_article_count) end_idx = wiki_article_count;
                    
                    for (int i = start_idx; i < end_idx; i++) {
                        char line[64];
                        char num_str[8];
                        simple_itoa(i + 1, num_str, 10);
                        
                        // Construire la ligne manuellement
                        int line_pos = 0;
                        // Copier le numéro
                        for (int j = 0; num_str[j] != '\0'; j++) {
                            line[line_pos++] = num_str[j];
                        }
                        line[line_pos++] = '.';
                        line[line_pos++] = ' ';
                        // Copier le titre
                        for (int j = 0; wiki_articles[i].title[j] != '\0' && line_pos < 60; j++) {
                            line[line_pos++] = wiki_articles[i].title[j];
                        }
                        line[line_pos] = '\0';
                        
                        print_at(line, VGA_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK), 7, 7 + (i - start_idx));
                    }
                    
                    // Instructions de navigation
                    if (total_pages > 1) {
                        show_message("Numero article + ENTREE | N=page suivante | P=precedente | Q=retour", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
                    } else {
                        show_message("Tapez le numero d'un article (puis ENTREE) ou Q pour retour", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
                    }
                    
                    // Gestion des commandes
                    int nav_key = getkey();
                    
                    if (nav_key == 'n' || nav_key == 'N') {
                        // Page suivante
                        if (current_page < total_pages - 1) {
                            current_page++;
                        }
                        continue;
                    }
                    else if (nav_key == 'p' || nav_key == 'P') {
                        // Page précédente
                        if (current_page > 0) {
                            current_page--;
                        }
                        continue;
                    }
                    else if (nav_key == 'q' || nav_key == 'Q') {
                        // Quitter
                        break;
                    }
                    else if (nav_key >= '0' && nav_key <= '9') {
                        // Début de saisie d'un numéro - traiter comme avant
                        // Réafficher la zone de saisie
                        print_at("Numero: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK), 5, 20);
                        terminal_putentryat(nav_key, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 13, 20);
                        
                        // Continuer la saisie depuis le premier chiffre saisi
                        char input_buffer[8];
                        input_buffer[0] = nav_key;
                        int input_len = 1;
                        
                        while (1) {
                            int key = getkey();
                            
                            if (key == '\n' || key == 13) {
                                // Entrée - valider la saisie
                                if (input_len > 0) {
                                    input_buffer[input_len] = '\0';
                                    
                                    // Convertir en nombre
                                    int article_id = 0;
                                    for (int i = 0; i < input_len; i++) {
                                        if (input_buffer[i] >= '0' && input_buffer[i] <= '9') {
                                            article_id = article_id * 10 + (input_buffer[i] - '0');
                                        } else {
                                            article_id = -1;
                                            break;
                                        }
                                    }
                                    
                                    if (article_id >= 1 && article_id <= wiki_article_count) {
                                        // Afficher l'article (index - 1 car les articles commencent à 0)
                                        int idx = article_id - 1;
                                        wiki_draw_screen();
                                        print_at(wiki_articles[idx].title, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 5, 5);
                                        
                                        // Afficher le contenu sur plusieurs lignes
                                        int y = 7;
                                        int x = 5;
                                        for (int i = 0; wiki_articles[idx].content[i] != '\0' && y < 20; i++) {
                                            char c = wiki_articles[idx].content[i];
                                            if (c == ' ' && x > 70) {
                                                y++;
                                                x = 5;
                                            } else {
                                                terminal_putentryat(c, VGA_COLOR(COLOR_WHITE, COLOR_BLACK), x, y);
                                                x++;
                                                if (x > 75) {
                                                    y++;
                                                    x = 5;
                                                }
                                            }
                                        }
                                        
                                        show_message("Appuyez sur une touche pour revenir", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
                                        getkey();
                                    } else {
                                        show_message("Numero d'article invalide!", VGA_COLOR(COLOR_RED, COLOR_BLACK));
                                        getkey();
                                    }
                                }
                                break;
                            }
                            else if (key == 'q' || key == 'Q') {
                                break;
                            }
                            else if (key == '\b' && input_len > 0) {
                                // Backspace
                                input_len--;
                                terminal_putentryat(' ', VGA_COLOR(COLOR_WHITE, COLOR_BLACK), 13 + input_len, 20);
                            }
                            else if (key >= '0' && key <= '9' && input_len < 6) {
                                // Chiffre
                                input_buffer[input_len] = key;
                                terminal_putentryat(key, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 13 + input_len, 20);
                                input_len++;
                            }
                        }
                    }
                }
                break;
            }
                
            
            case '2': {
                // Recherche
                wiki_draw_screen();
                print_at("=== RECHERCHE ===", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 5, 5);
                print_at("Tapez votre terme de recherche:", VGA_COLOR(COLOR_WHITE, COLOR_BLACK), 5, 7);
                
                // Saisie du terme de recherche
                char search_buffer[32];
                int search_len = 0;
                int search_x = 5, search_y = 9;
                
                print_at("Recherche: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK), search_x, search_y);
                show_message("Tapez votre recherche puis ENTREE, Q pour retour", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
                
                while (1) {
                    int key = getkey();
                    
                    if (key == '\n' || key == 13) {
                        // Entrée - effectuer la recherche
                        if (search_len > 0) {
                            search_buffer[search_len] = '\0';
                            
                            // Effectuer la recherche
                            int results[10];
                            int found = wiki_search(search_buffer, results, 10);
                            
                            // Afficher les résultats
                            wiki_draw_screen();
                            print_at("=== RESULTATS DE RECHERCHE ===", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 5, 5);
                            
                            char search_info[64];
                            int pos = 0;
                            char* prefix = "Recherche pour: ";
                            for (int i = 0; prefix[i] != '\0'; i++) search_info[pos++] = prefix[i];
                            for (int i = 0; i < search_len && pos < 60; i++) search_info[pos++] = search_buffer[i];
                            search_info[pos] = '\0';
                            print_at(search_info, VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 5, 7);
                            
                            if (found == 0) {
                                print_at("Aucun resultat trouve.", VGA_COLOR(COLOR_RED, COLOR_BLACK), 5, 9);
                            } else {
                                char found_info[32];
                                char found_str[8];
                                simple_itoa(found, found_str, 10);
                                pos = 0;
                                char* prefix2 = "Resultats trouves: ";
                                for (int i = 0; prefix2[i] != '\0'; i++) found_info[pos++] = prefix2[i];
                                for (int i = 0; found_str[i] != '\0'; i++) found_info[pos++] = found_str[i];
                                found_info[pos] = '\0';
                                print_at(found_info, VGA_COLOR(COLOR_GREEN, COLOR_BLACK), 5, 9);
                                
                                // Afficher les résultats
                                int max_display = (found > 8) ? 8 : found;
                                for (int i = 0; i < max_display; i++) {
                                    char result_line[64];
                                    char num_str[8];
                                    simple_itoa(i + 1, num_str, 10);
                                    
                                    pos = 0;
                                    // Numéro
                                    for (int j = 0; num_str[j] != '\0'; j++) result_line[pos++] = num_str[j];
                                    result_line[pos++] = '.';
                                    result_line[pos++] = ' ';
                                    // Titre
                                    for (int j = 0; wiki_articles[results[i]].title[j] != '\0' && pos < 60; j++) {
                                        result_line[pos++] = wiki_articles[results[i]].title[j];
                                    }
                                    result_line[pos] = '\0';
                                    
                                    print_at(result_line, VGA_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK), 7, 11 + i);
                                }
                                
                                show_message("Tapez le numero d'un resultat (puis ENTREE) ou Q pour retour", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
                                
                                // Saisie du choix de résultat
                                char result_buffer[8];
                                int result_len = 0;
                                int result_x = 5, result_y = 20;
                                
                                print_at("Numero: ", VGA_COLOR(COLOR_WHITE, COLOR_BLACK), result_x, result_y);
                                
                                while (1) {
                                    int result_key = getkey();
                                    
                                    if (result_key == '\n' || result_key == 13) {
                                        if (result_len > 0) {
                                            result_buffer[result_len] = '\0';
                                            
                                            // Convertir en nombre
                                            int choice = 0;
                                            for (int i = 0; i < result_len; i++) {
                                                if (result_buffer[i] >= '0' && result_buffer[i] <= '9') {
                                                    choice = choice * 10 + (result_buffer[i] - '0');
                                                } else {
                                                    choice = -1;
                                                    break;
                                                }
                                            }
                                            
                                            if (choice >= 1 && choice <= found) {
                                                // Afficher l'article sélectionné
                                                int article_idx = results[choice - 1];
                                                wiki_draw_screen();
                                                print_at(wiki_articles[article_idx].title, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 5, 5);
                                                
                                                // Afficher le contenu
                                                int y = 7;
                                                int x = 5;
                                                for (int i = 0; wiki_articles[article_idx].content[i] != '\0' && y < 20; i++) {
                                                    char c = wiki_articles[article_idx].content[i];
                                                    if (c == ' ' && x > 70) {
                                                        y++;
                                                        x = 5;
                                                    } else {
                                                        terminal_putentryat(c, VGA_COLOR(COLOR_WHITE, COLOR_BLACK), x, y);
                                                        x++;
                                                        if (x > 75) {
                                                            y++;
                                                            x = 5;
                                                        }
                                                    }
                                                }
                                                
                                                show_message("Appuyez sur une touche pour revenir", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
                                                getkey();
                                            } else {
                                                show_message("Numero invalide!", VGA_COLOR(COLOR_RED, COLOR_BLACK));
                                                getkey();
                                            }
                                        }
                                        break;
                                    }
                                    else if (result_key == 'q' || result_key == 'Q') {
                                        break;
                                    }
                                    else if (result_key == '\b' && result_len > 0) {
                                        result_len--;
                                        terminal_putentryat(' ', VGA_COLOR(COLOR_WHITE, COLOR_BLACK), result_x + 8 + result_len, result_y);
                                    }
                                    else if (result_key >= '0' && result_key <= '9' && result_len < 6) {
                                        result_buffer[result_len] = result_key;
                                        terminal_putentryat(result_key, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), result_x + 8 + result_len, result_y);
                                        result_len++;
                                    }
                                }
                            }
                            
                            show_message("Appuyez sur une touche pour continuer", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
                            getkey();
                        }
                        break;
                    }
                    else if (key == 'q' || key == 'Q') {
                        break;
                    }
                    else if (key == '\b' && search_len > 0) {
                        // Backspace
                        search_len--;
                        terminal_putentryat(' ', VGA_COLOR(COLOR_WHITE, COLOR_BLACK), search_x + 11 + search_len, search_y);
                    }
                    else if (key >= 32 && key <= 126 && search_len < 30) {
                        // Caractère imprimable
                        search_buffer[search_len] = key;
                        terminal_putentryat(key, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), search_x + 11 + search_len, search_y);
                        search_len++;
                    }
                }
                break;
            }
            
            case '3': {
                // Article aléatoire
                if (wiki_article_count > 0) {
                    // Utiliser le tick pour un pseudo-aléatoire
                    extern uint32_t tick;
                    int random_id = tick % wiki_article_count;
                    
                    wiki_draw_screen();
                    print_at("Article aleatoire:", VGA_COLOR(COLOR_WHITE, COLOR_BLACK), 5, 5);
                    print_at(wiki_articles[random_id].title, VGA_COLOR(COLOR_YELLOW, COLOR_BLACK), 5, 7);
                    
                    // Afficher le contenu sur plusieurs lignes
                    int y = 9;
                    int x = 5;
                    for (int i = 0; wiki_articles[random_id].content[i] != '\0' && y < 20; i++) {
                        char c = wiki_articles[random_id].content[i];
                        if (c == ' ' && x > 70) {
                            y++;
                            x = 5;
                        } else {
                            terminal_putentryat(c, VGA_COLOR(COLOR_WHITE, COLOR_BLACK), x, y);
                            x++;
                            if (x > 75) {
                                y++;
                                x = 5;
                            }
                        }
                    }
                    
                    show_message("Appuyez sur une touche pour revenir", VGA_COLOR(COLOR_YELLOW, COLOR_BLACK));
                    getkey();
                }
                break;
            }
            
            case 'q':
            case 'Q':
                return;
        }
    }
}

// Interface utilisateur pour la commande wiki - Lance directement l'UI
void do_wiki(char* args) {
    // Ignorer les arguments et lancer directement l'interface graphique
    (void)args; // Éviter warning unused parameter
    
    wiki_interactive_ui();
    clear_screen();
    terminal_writestring("Retour au shell principal.\n");
}