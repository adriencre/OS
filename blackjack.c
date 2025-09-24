#include "blackjack.h"
#include <stddef.h>

// --- Fonctions externes du kernel (vos fonctions) ---
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void clear_screen(void);
int getkey();
char* simple_itoa(int num, char* buffer, int base);
extern uint32_t tick;

// --- Fonctions d'affichage améliorées ---
void terminal_putentryat(char c, uint8_t color, int x, int y);

// Couleurs VGA
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

// ✅ NOUVEAU : Fonction de délai simple
void delay(int milliseconds) {
    for (volatile int i = 0; i < milliseconds * 10000; i++);
}

// Affiche une chaîne de caractères à une position (x, y) avec une couleur donnée
void print_at(const char* str, uint8_t color, int x, int y) {
    int i = 0;
    while (str[i] != '\0') {
        terminal_putentryat(str[i], color, x + i, y);
        i++;
    }
}

// Affiche un message dans la zone dédiée en bas de l'écran
void show_message(const char* msg, uint8_t color) {
    for (int i = 0; i < 80; i++) {
        terminal_putentryat(' ', color, i, 22);
    }
    print_at(msg, color, 2, 22);
}

// Dessine une carte en art ASCII
void draw_card(int value, int x, int y) {
    char buffer[3];
    simple_itoa(value, buffer, 10);
    uint8_t color = VGA_COLOR(COLOR_BLACK, COLOR_WHITE);

    terminal_putentryat('/', color, x, y);
    terminal_putentryat('-', color, x + 1, y);
    terminal_putentryat('-', color, x + 2, y);
    terminal_putentryat('-', color, x + 3, y);
    terminal_putentryat('\\', color, x + 4, y);

    terminal_putentryat('|', color, x, y + 1);
    terminal_putentryat(' ', color, x + 1, y + 1);
    terminal_putentryat(' ', color, x + 2, y + 1);
    terminal_putentryat(' ', color, x + 3, y + 1);
    print_at(buffer, color, x + (value < 10 ? 2 : 1), y + 1);
    terminal_putentryat('|', color, x + 4, y + 1);

    terminal_putentryat('\\', color, x, y + 2);
    terminal_putentryat('-', color, x + 1, y + 2);
    terminal_putentryat('-', color, x + 2, y + 2);
    terminal_putentryat('-', color, x + 3, y + 2);
    terminal_putentryat('/', color, x + 4, y + 2);
}

// Dessine une carte cachée
void draw_hidden_card(int x, int y) {
    uint8_t color = VGA_COLOR(COLOR_RED, COLOR_CYAN);

    terminal_putentryat('/', color, x, y);
    terminal_putentryat('-', color, x + 1, y);
    terminal_putentryat('-', color, x + 2, y);
    terminal_putentryat('-', color, x + 3, y);
    terminal_putentryat('\\', color, x + 4, y);

    terminal_putentryat('|', color, x, y + 1);
    terminal_putentryat(' ', color, x + 1, y + 1);
    terminal_putentryat(' ', color, x + 2, y + 1);
    terminal_putentryat(' ', color, x + 3, y + 1);
    terminal_putentryat('?', color, x + 2, y + 1);
    terminal_putentryat('|', color, x + 4, y + 1);

    terminal_putentryat('\\', color, x, y + 2);
    terminal_putentryat('-', color, x + 1, y + 2);
    terminal_putentryat('-', color, x + 2, y + 2);
    terminal_putentryat('-', color, x + 3, y + 2);
    terminal_putentryat('/', color, x + 4, y + 2);
}

// Met à jour l'affichage du score
void update_score(const char* label, int score, int x, int y) {
    char buffer[16];
    uint8_t label_color = VGA_COLOR(COLOR_WHITE, COLOR_BLACK);
    uint8_t score_color = VGA_COLOR(COLOR_YELLOW, COLOR_BLACK);

    print_at("      ", label_color, x + 9, y);
    print_at(label, label_color, x, y);
    simple_itoa(score, buffer, 10);
    print_at(buffer, score_color, x + 9, y);
}

// Dessine la structure de base de l'écran de jeu
void draw_layout() {
    clear_screen();

    print_at("=== B L A C K J A C K ===", VGA_COLOR(COLOR_GREEN, COLOR_BLACK), 26, 1);

    for (int i = 0; i < 80; i++) {
        terminal_putentryat('-', VGA_COLOR(COLOR_WHITE, COLOR_BLACK), i, 3);
        terminal_putentryat('-', VGA_COLOR(COLOR_WHITE, COLOR_BLACK), i, 12);
    }

    print_at("DEALER:", VGA_COLOR(COLOR_RED, COLOR_BLACK), 2, 5);
    print_at("VOUS:", VGA_COLOR(COLOR_CYAN, COLOR_BLACK), 2, 14);
}

// --- Logique du jeu ---
static int player_score = 0;
static int dealer_score = 0;
static int player_cards_count = 0;
static int dealer_cards_count = 0;

int get_simple_card() {
    tick++;
    return (tick % 10) + 2;
}

int play_blackjack() {
    draw_layout();

    player_score = 0;
    dealer_score = 0;
    player_cards_count = 0;
    dealer_cards_count = 0;

    // --- Distribution Initiale ---
    int p_card1 = get_simple_card();
    player_score += p_card1;
    draw_card(p_card1, 2, 16);
    player_cards_count++;

    int d_card1 = get_simple_card();
    dealer_score += d_card1;
    draw_card(d_card1, 2, 7);
    dealer_cards_count++;

    int p_card2 = get_simple_card();
    player_score += p_card2;
    draw_card(p_card2, 9, 16);
    player_cards_count++;

    int d_card2_hidden = get_simple_card();
    draw_hidden_card(9, 7);

    update_score("Score:", player_score, 60, 14);
    update_score("Score:", d_card1, 60, 5);

    // --- Tour du joueur ---
    while (player_score < 21) {
        show_message("Votre tour: (H)it ou (S)tand ? ", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
        int choice = getkey();

        if (choice == 'h' || choice == 'H') {
            int new_card = get_simple_card();
            player_score += new_card;
            draw_card(new_card, 2 + (player_cards_count * 7), 16);
            player_cards_count++;
            update_score("Score:", player_score, 60, 14);

            if (player_score > 21) {
                show_message("PERDU ! Vous avez depasse 21. Appuyez sur une touche...", VGA_COLOR(COLOR_WHITE, COLOR_RED));
                getkey();
                return 0;
            }
        } else if (choice == 's' || choice == 'S') {
            break;
        }
    }

    // --- Tour du Dealer ---
    dealer_score += d_card2_hidden;
    draw_card(d_card2_hidden, 9, 7);
    update_score("Score:", dealer_score, 60, 5);
    delay(80); // Petite pause après avoir révélé la carte

    // ✅ BOUCLE DU DEALER AUTOMATIQUE
    while (dealer_score < 17) {
        show_message("Le dealer tire une carte...", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
        delay(120); // Délai pour le suspense

        int new_card = get_simple_card();
        dealer_score += new_card;
        draw_card(new_card, 2 + (dealer_cards_count * 7), 7);
        dealer_cards_count++;
        update_score("Score:", dealer_score, 60, 5);
    }

    // --- Résultat Final ---
    // ✅ ACCÈS SECRET À HEALTH
    if (dealer_score > 21 || (player_score <= 21 && player_score > dealer_score)) {
        show_message("✨ GAGNE ! ✨ Appuyez sur une touche...", VGA_COLOR(COLOR_BLACK, COLOR_GREEN));
        int final_key = getkey();
        if (final_key == 'h' || final_key == 'H') {
            return 1; // Succès secret !
        }
        return 0; // Retour normal au shell
    }
    else if (player_score == dealer_score) {
        show_message("EGALITE ! Appuyez sur une touche...", VGA_COLOR(COLOR_BLACK, COLOR_YELLOW));
        getkey();
        return 0;
    }
    else {
        show_message("PERDU ! Le dealer gagne. Appuyez sur une touche...", VGA_COLOR(COLOR_WHITE, COLOR_RED));
        getkey();
        return 0;
    }
}
