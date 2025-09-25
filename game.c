// game.c - Version améliorée du Snake
#include "game.h"
#include "common.h"
#include "keyboard.h"

// Déclarations de fonctions du kernel
void terminal_putchar_at(char c, int x, int y);
void terminal_writestring(const char* data);
void clear_screen();
char* simple_itoa(int num, char* buffer, int base);
extern uint32_t tick;

// --- Snake amélioré ---
#define BOARD_WIDTH 40
#define BOARD_HEIGHT 20
#define MAX_SNAKE_LENGTH 50

static int snake_x[MAX_SNAKE_LENGTH];
static int snake_y[MAX_SNAKE_LENGTH];
static int snake_length = 3;
static int food_x = 20;
static int food_y = 10;
static int direction = 0; // 0=right, 1=down, 2=left, 3=up
static int game_over = 0;
static int score = 0;
static int high_score = 0;
static int speed = 15;
static int level = 1;
static int food_count = 0;

// --- Fonctions améliorées ---

void init_snake() {
    snake_length = 3;
    snake_x[0] = 20; snake_y[0] = 10;
    snake_x[1] = 19; snake_y[1] = 10;
    snake_x[2] = 18; snake_y[2] = 10;
    direction = 0;
    game_over = 0;
    score = 0;
    level = 1;
    speed = 15;
    food_count = 0;
    food_x = 25;
    food_y = 12;
}

void draw_border() {
    // Dessiner les bordures
    for (int x = 0; x < BOARD_WIDTH; x++) {
        terminal_putchar_at('#', x, 0);
        terminal_putchar_at('#', x, BOARD_HEIGHT - 1);
    }
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        terminal_putchar_at('#', 0, y);
        terminal_putchar_at('#', BOARD_WIDTH - 1, y);
    }
}

void draw_ui() {
    // Afficher l'interface utilisateur
    terminal_writestring("=== SNAKE ENHANCED ===");
    
    // Score
    terminal_writestring(" Score: ");
    char score_str[10];
    simple_itoa(score, score_str, 10);
    terminal_writestring(score_str);
    
    // High Score
    terminal_writestring(" | High: ");
    char high_str[10];
    simple_itoa(high_score, high_str, 10);
    terminal_writestring(high_str);
    
    // Level
    terminal_writestring(" | Level: ");
    char level_str[5];
    simple_itoa(level, level_str, 10);
    terminal_writestring(level_str);
    
    // Instructions
    terminal_writestring(" | ZQSD/FLECHES + ESC");
}

void place_food() {
    // Placement simple et sûr de la nourriture
    food_x = (tick % (BOARD_WIDTH - 2)) + 1;
    food_y = (tick % (BOARD_HEIGHT - 2)) + 1;
    
    // Vérification simple (pas de boucle complexe)
    int attempts = 0;
    while (attempts < 20) {
        int on_snake = 0;
        for (int i = 0; i < snake_length; i++) {
            if (snake_x[i] == food_x && snake_y[i] == food_y) {
                on_snake = 1;
                break;
            }
        }
        
        if (!on_snake) break;
        
        // Déplacer la nourriture
        food_x = (food_x + 3) % (BOARD_WIDTH - 2) + 1;
        food_y = (food_y + 2) % (BOARD_HEIGHT - 2) + 1;
        attempts++;
    }
}

void enhanced_loop() {
    uint32_t last = tick;
    
    while (!game_over) {
        int key = check_key();
        if (key == 27) break; // ESC
        
        // Contrôles avec les flèches directionnelles
        if (key == 0x48 && direction != 1) direction = 3; // Flèche haut
        if (key == 0x50 && direction != 3) direction = 1; // Flèche bas
        if (key == 0x4B && direction != 0) direction = 2; // Flèche gauche
        if (key == 0x4D && direction != 2) direction = 0; // Flèche droite
        
        // Contrôles AZERTY (ZQSD)
        if (key == 'z' && direction != 1) direction = 3; // Z = haut
        if (key == 's' && direction != 3) direction = 1; // S = bas
        if (key == 'q' && direction != 0) direction = 2; // Q = gauche
        if (key == 'd' && direction != 2) direction = 0; // D = droite
        
        // Contrôles alternatifs QWERTY (WASD) pour compatibilité
        if (key == 'w' && direction != 1) direction = 3;
        if (key == 's' && direction != 3) direction = 1;
        if (key == 'a' && direction != 0) direction = 2;
        if (key == 'd' && direction != 2) direction = 0;
        
        if (tick >= last + speed) {
            last = tick;
            
            // Bouger le corps
            for (int i = snake_length - 1; i > 0; i--) {
                snake_x[i] = snake_x[i-1];
                snake_y[i] = snake_y[i-1];
            }
            
            // Bouger la tête
            if (direction == 0) snake_x[0]++;
            if (direction == 1) snake_y[0]++;
            if (direction == 2) snake_x[0]--;
            if (direction == 3) snake_y[0]--;
            
            // Collision murs
            if (snake_x[0] <= 0 || snake_x[0] >= BOARD_WIDTH - 1 || 
                snake_y[0] <= 0 || snake_y[0] >= BOARD_HEIGHT - 1) {
                game_over = 1;
                break;
            }
            
            // Collision corps
            for (int i = 1; i < snake_length; i++) {
                if (snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i]) {
                    game_over = 1;
                    break;
                }
            }
            
            if (game_over) break;
            
            // Manger nourriture
            if (snake_x[0] == food_x && snake_y[0] == food_y) {
                score += 10;
                food_count++;
                snake_length++;
                
                // Augmenter le niveau (plus conservateur)
                if (food_count % 10 == 0) { // Tous les 10 points au lieu de 5
                    level++;
                    if (speed > 8) speed -= 1; // Réduction plus douce
                }
                
                // Mettre à jour le high score
                if (score > high_score) {
                    high_score = score;
                }
                
                place_food();
            }
            
            // Dessiner le jeu
            clear_screen();
            draw_ui();
            draw_border();
            
            // Dessiner le serpent avec des caractères améliorés
            terminal_putchar_at('@', snake_x[0], snake_y[0]); // Tête
            for (int i = 1; i < snake_length; i++) {
                terminal_putchar_at('o', snake_x[i], snake_y[i]); // Corps
            }
            
            // Dessiner la nourriture avec un caractère spécial
            terminal_putchar_at('*', food_x, food_y);
        }
    }
}

void show_game_over() {
    clear_screen();
    terminal_writestring("=== GAME OVER ===\n");
    terminal_writestring("Score final: ");
    char score_str[10];
    simple_itoa(score, score_str, 10);
    terminal_writestring(score_str);
    terminal_writestring("\n");
    
    terminal_writestring("High Score: ");
    char high_str[10];
    simple_itoa(high_score, high_str, 10);
    terminal_writestring(high_str);
    terminal_writestring("\n");
    
    terminal_writestring("Niveau atteint: ");
    char level_str[5];
    simple_itoa(level, level_str, 10);
    terminal_writestring(level_str);
    terminal_writestring("\n");
    
    terminal_writestring("Longueur du serpent: ");
    char length_str[5];
    simple_itoa(snake_length, length_str, 10);
    terminal_writestring(length_str);
    terminal_writestring("\n");
    
    if (score == high_score && score > 0) {
        terminal_writestring("NOUVEAU RECORD !!!\n");
    }
    
    terminal_writestring("\nAppuyez sur une touche pour continuer...\n");
    getkey();
}

void play_snake() {
    init_snake();
    
    // Écran de démarrage
    clear_screen();
    terminal_writestring("=== SNAKE ENHANCED ===\n");
    terminal_writestring("Controles: ZQSD ou FLECHES pour bouger, ESC pour quitter\n");
    terminal_writestring("Objectif: Mangez la nourriture (*) pour grandir\n");
    terminal_writestring("Attention: Evitez les murs et votre propre corps!\n");
    terminal_writestring("Le jeu s'accelere tous les 10 points!\n");
    terminal_writestring("\nAppuyez sur une touche pour commencer...\n");
    getkey();
    
    // Lancer le jeu
    enhanced_loop();
    
    // Afficher l'écran de fin
    show_game_over();
}