// pong.c - Jeu de Pong simple et addictif
#include "pong.h"
#include "common.h"
#include "keyboard.h"

// Déclarations de fonctions du kernel
void terminal_putchar_at(char c, int x, int y);
void terminal_writestring(const char* data);
void clear_screen();
char* simple_itoa(int num, char* buffer, int base);
extern uint32_t tick;

// --- Constantes du jeu Pong ---
#define PONG_WIDTH 60
#define PONG_HEIGHT 20
#define PADDLE_HEIGHT 4
#define PADDLE_SPEED 1
#define BALL_SPEED 1

// --- Variables du jeu ---
static int left_paddle_y = 8;   // Position Y de la raquette gauche
static int right_paddle_y = 8;  // Position Y de la raquette droite
static int ball_x = 30;         // Position X de la balle
static int ball_y = 10;         // Position Y de la balle
static int ball_dx = 1;         // Direction X de la balle (-1 ou 1)
static int ball_dy = 1;         // Direction Y de la balle (-1 ou 1)
static int left_score = 0;      // Score du joueur gauche
static int right_score = 0;     // Score du joueur droit
static int game_mode = 1;       // 1 = 2 joueurs, 2 = contre IA
static int game_over = 0;       // État du jeu
static int winner = 0;          // 1 = gauche, 2 = droite
static int max_score = 5;       // Score maximum pour gagner

// --- Fonctions du jeu ---

void init_pong() {
    left_paddle_y = 8;
    right_paddle_y = 8;
    ball_x = 30;
    ball_y = 10;
    ball_dx = 1;
    ball_dy = 1;
    left_score = 0;
    right_score = 0;
    game_over = 0;
    winner = 0;
}

void draw_pong_border() {
    // Dessiner les bordures du terrain
    for (int x = 0; x < PONG_WIDTH; x++) {
        terminal_putchar_at('#', x, 0);
        terminal_putchar_at('#', x, PONG_HEIGHT - 1);
    }
    for (int y = 0; y < PONG_HEIGHT; y++) {
        terminal_putchar_at('#', 0, y);
        terminal_putchar_at('#', PONG_WIDTH - 1, y);
    }
    
    // Ligne centrale
    for (int y = 1; y < PONG_HEIGHT - 1; y += 2) {
        terminal_putchar_at('|', PONG_WIDTH / 2, y);
    }
}

void draw_paddles() {
    // Raquette gauche
    for (int i = 0; i < PADDLE_HEIGHT; i++) {
        terminal_putchar_at('|', 1, left_paddle_y + i);
    }
    
    // Raquette droite
    for (int i = 0; i < PADDLE_HEIGHT; i++) {
        terminal_putchar_at('|', PONG_WIDTH - 2, right_paddle_y + i);
    }
}

void draw_ball() {
    terminal_putchar_at('O', ball_x, ball_y);
}

void draw_pong_ui() {
    // Afficher le titre
    terminal_writestring("=== PONG GAME ===");
    
    // Score gauche
    terminal_writestring(" Joueur: ");
    char score_str[5];
    simple_itoa(left_score, score_str, 10);
    terminal_writestring(score_str);
    
    // Score droite
    terminal_writestring(" | IA: ");
    simple_itoa(right_score, score_str, 10);
    terminal_writestring(score_str);
    
    // Mode de jeu
    if (game_mode == 1) {
        terminal_writestring(" | Mode: 2 Joueurs");
    } else {
        terminal_writestring(" | Mode: vs IA");
    }
    
    // Instructions
    terminal_writestring(" | Z/S vs Q/W + ESC");
}

void move_ball() {
    ball_x += ball_dx;
    ball_y += ball_dy;
    
    // Collision avec les murs haut/bas
    if (ball_y <= 1 || ball_y >= PONG_HEIGHT - 2) {
        ball_dy = -ball_dy;
    }
    
    // Collision avec la raquette gauche
    if (ball_x == 2 && ball_y >= left_paddle_y && ball_y < left_paddle_y + PADDLE_HEIGHT) {
        ball_dx = 1;
        // Angle de rebond basé sur la position de la balle sur la raquette
        int paddle_center = left_paddle_y + PADDLE_HEIGHT / 2;
        if (ball_y < paddle_center) ball_dy = -1;
        else if (ball_y > paddle_center) ball_dy = 1;
        else ball_dy = 0;
    }
    
    // Collision avec la raquette droite
    if (ball_x == PONG_WIDTH - 3 && ball_y >= right_paddle_y && ball_y < right_paddle_y + PADDLE_HEIGHT) {
        ball_dx = -1;
        // Angle de rebond basé sur la position de la balle sur la raquette
        int paddle_center = right_paddle_y + PADDLE_HEIGHT / 2;
        if (ball_y < paddle_center) ball_dy = -1;
        else if (ball_y > paddle_center) ball_dy = 1;
        else ball_dy = 0;
    }
    
    // But marqué
    if (ball_x <= 0) {
        right_score++;
        if (right_score >= max_score) {
            game_over = 1;
            winner = 2;
        } else {
            // Reset de la balle
            ball_x = 30;
            ball_y = 10;
            ball_dx = 1;
            ball_dy = 1;
        }
    } else if (ball_x >= PONG_WIDTH - 1) {
        left_score++;
        if (left_score >= max_score) {
            game_over = 1;
            winner = 1;
        } else {
            // Reset de la balle
            ball_x = 30;
            ball_y = 10;
            ball_dx = -1;
            ball_dy = 1;
        }
    }
}

void ai_move() {
    if (game_mode == 2) {
        // IA simple : suivre la balle
        int paddle_center = right_paddle_y + PADDLE_HEIGHT / 2;
        
        if (ball_y < paddle_center && right_paddle_y > 1) {
            right_paddle_y -= PADDLE_SPEED;
        } else if (ball_y > paddle_center && right_paddle_y < PONG_HEIGHT - PADDLE_HEIGHT - 1) {
            right_paddle_y += PADDLE_SPEED;
        }
    }
}

void handle_input() {
    int key = check_key();
    if (key == 27) { // ESC
        game_over = 1;
        return;
    }
    
    // Contrôles du joueur gauche (Z/S)
    if (key == 'z' && left_paddle_y > 1) {
        left_paddle_y -= PADDLE_SPEED;
    }
    if (key == 's' && left_paddle_y < PONG_HEIGHT - PADDLE_HEIGHT - 1) {
        left_paddle_y += PADDLE_SPEED;
    }
    
    // Contrôles du joueur droit (Q/W) - seulement en mode 2 joueurs
    if (game_mode == 1) {
        if (key == 'q' && right_paddle_y > 1) {
            right_paddle_y -= PADDLE_SPEED;
        }
        if (key == 'w' && right_paddle_y < PONG_HEIGHT - PADDLE_HEIGHT - 1) {
            right_paddle_y += PADDLE_SPEED;
        }
    }
}

void game_loop() {
    uint32_t last_update = tick;
    uint32_t game_speed = 8; // Vitesse du jeu (plus petit = plus rapide)
    
    while (!game_over) {
        handle_input();
        
        if (tick >= last_update + game_speed) {
            last_update = tick;
            
            move_ball();
            ai_move();
            
            // Dessiner le jeu
            clear_screen();
            draw_pong_ui();
            draw_pong_border();
            draw_paddles();
            draw_ball();
        }
    }
}

void show_pong_game_over() {
    clear_screen();
    terminal_writestring("=== GAME OVER ===\n");
    
    if (winner == 1) {
        terminal_writestring("Joueur gagne !\n");
    } else if (winner == 2) {
        if (game_mode == 1) {
            terminal_writestring("Joueur 2 gagne !\n");
        } else {
            terminal_writestring("IA gagne !\n");
        }
    }
    
    terminal_writestring("Score final: ");
    char score_str[5];
    simple_itoa(left_score, score_str, 10);
    terminal_writestring(score_str);
    terminal_writestring(" - ");
    simple_itoa(right_score, score_str, 10);
    terminal_writestring(score_str);
    terminal_writestring("\n");
    
    terminal_writestring("\nAppuyez sur une touche pour continuer...\n");
    getkey();
}

void select_game_mode() {
    clear_screen();
    terminal_writestring("=== SELECTION DU MODE ===\n");
    terminal_writestring("1. 2 Joueurs (Z/S vs Q/W)\n");
    terminal_writestring("2. Contre l'IA (Z/S vs IA)\n");
    terminal_writestring("\nChoisissez un mode (1 ou 2): ");
    
    while (1) {
        int key = getkey();
        if (key == '1') {
            game_mode = 1;
            break;
        } else if (key == '2') {
            game_mode = 2;
            break;
        } else if (key == 27) { // ESC
            return;
        }
    }
}

void play_pong() {
    select_game_mode();
    
    // Écran de démarrage
    clear_screen();
    terminal_writestring("=== PONG GAME ===\n");
    if (game_mode == 1) {
        terminal_writestring("Mode: 2 Joueurs\n");
        terminal_writestring("Joueur 1: Z (haut) / S (bas)\n");
        terminal_writestring("Joueur 2: Q (haut) / W (bas)\n");
    } else {
        terminal_writestring("Mode: Contre l'IA\n");
        terminal_writestring("Joueur: Z (haut) / S (bas)\n");
        terminal_writestring("IA: Mouvement automatique\n");
    }
    terminal_writestring("Objectif: Marquer 5 points pour gagner\n");
    terminal_writestring("ESC pour quitter\n");
    terminal_writestring("\nAppuyez sur une touche pour commencer...\n");
    getkey();
    
    // Initialiser et lancer le jeu
    init_pong();
    game_loop();
    
    // Afficher l'écran de fin
    show_pong_game_over();
}
