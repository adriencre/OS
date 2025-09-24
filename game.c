// game.c
#include "game.h"
#include "common.h"
#include "keyboard.h"

// Déclarations de fonctions du kernel dont nous avons besoin
void terminal_putchar_at(char c, int x, int y);
void terminal_writestring(const char* data);
void clear_screen();
extern uint32_t tick; // Le compteur de temps global

// --- Constantes et état du jeu ---
#define BOARD_WIDTH 78
#define BOARD_HEIGHT 23
#define MAX_SNAKE_LENGTH 100

typedef struct {
    int x, y;
} Point;

Point snake[MAX_SNAKE_LENGTH];
int snake_length;
Point food;
enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
enum Direction direction;
int game_over;
int score;

// --- Fonctions du jeu ---

void place_food() {
    // "Aléatoire" simple basé sur les ticks
    food.x = (tick % (BOARD_WIDTH - 2)) + 1;
    food.y = (tick % (BOARD_HEIGHT - 2)) + 1;
}

void snake_init() {
    snake_length = 3;
    snake[0] = (Point){BOARD_WIDTH / 2, BOARD_HEIGHT / 2};
    snake[1] = (Point){BOARD_WIDTH / 2 - 1, BOARD_HEIGHT / 2};
    snake[2] = (Point){BOARD_WIDTH / 2 - 2, BOARD_HEIGHT / 2};
    direction = DIR_RIGHT;
    game_over = 0;
    score = 0;
    place_food();
}

void draw_board() {
    clear_screen();
    for (int x = 0; x < BOARD_WIDTH; x++) {
        terminal_putchar_at('#', x, 0);
        terminal_putchar_at('#', x, BOARD_HEIGHT - 1);
    }
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        terminal_putchar_at('#', 0, y);
        terminal_putchar_at('#', BOARD_WIDTH - 1, y);
    }
}

void game_loop() {
    uint32_t last_update = tick;
    int speed = 10; // 1 mouvement tous les 10 ticks (100ms)

    while (!game_over) {
        // 1. Gérer l'entrée utilisateur (non-bloquant)
        int key = check_key();
        if ((key == 'z' || key == 'Z') && direction != DIR_DOWN) direction = DIR_UP;
        else if ((key == 's' || key == 'S') && direction != DIR_UP) direction = DIR_DOWN;
        else if ((key == 'q' || key == 'Q') && direction != DIR_RIGHT) direction = DIR_LEFT;
        else if ((key == 'd' || key == 'D') && direction != DIR_LEFT) direction = DIR_RIGHT;

        // 2. Mettre à jour l'état du jeu à intervalle régulier
        if (tick >= last_update + speed) {
            last_update = tick;

            // Efface la queue du serpent
            terminal_putchar_at(' ', snake[snake_length - 1].x, snake[snake_length - 1].y);

            // Fait avancer le corps
            for (int i = snake_length - 1; i > 0; i--) {
                snake[i] = snake[i - 1];
            }

            // Fait avancer la tête
            if (direction == DIR_UP) snake[0].y--;
            if (direction == DIR_DOWN) snake[0].y++;
            if (direction == DIR_LEFT) snake[0].x--;
            if (direction == DIR_RIGHT) snake[0].x++;

            // 3. Vérifier les collisions
            if (snake[0].x == food.x && snake[0].y == food.y) {
                if (snake_length < MAX_SNAKE_LENGTH) {
                    snake_length++;
                    score++;
                }
                place_food();
            }

            if (snake[0].x <= 0 || snake[0].x >= BOARD_WIDTH - 1 || snake[0].y <= 0 || snake[0].y >= BOARD_HEIGHT - 1) {
                game_over = 1;
            }

            for (int i = 1; i < snake_length; i++) {
                if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                    game_over = 1;
                }
            }

            // 4. Redessiner
            terminal_putchar_at('O', snake[0].x, snake[0].y);
            for (int i = 1; i < snake_length; i++) {
                terminal_putchar_at('o', snake[i].x, snake[i].y);
            }
            terminal_putchar_at('*', food.x, food.y);
        }
    }
}

void play_snake() {
    draw_board();
    snake_init();
    game_loop();
    
    // Écran de fin
    terminal_writestring("GAME OVER - Score: ");
    // Afficher le score (nécessite itoa, que nous devons rendre accessible)
    // Pour l'instant, on attend une touche pour revenir.
    getkey();
}