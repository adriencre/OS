// keyboard.c
#include "keyboard.h"
#include "common.h"

enum KEYCODE {
    KEY_UNKNOWN = 0, KEY_ENTER = '\n', KEY_BACKSPACE = '\b',
    KEY_UP = 257, KEY_DOWN = 258, KEY_LEFT = 259, KEY_RIGHT = 260,
    KEY_DELETE = 261, KEY_HOME = 262, KEY_END = 263,
};

static int shift_pressed = 0;
const char scancode_map_azerty_normal[] = {
    0, 27, '&', 233, '"', '\'', '(', '-', 232, '_', 231, 224, ')', '=', KEY_BACKSPACE,
    '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', KEY_ENTER,
    0, 'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 249, '*',
    0, '<', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!', 0, 0, 0, ' ', 0
};
const char scancode_map_azerty_shifted[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 248, '+', KEY_BACKSPACE,
    '\t', 'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 249, 156, KEY_ENTER,
    0, 'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '%', '`',
    0, '>', 'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', 21, 0, 0, 0, ' ', 0
};

// Version BLOQUANTE (pour le shell)
int getkey() {
    while(1) {
        if (inb(0x64) & 0x1) {
            uint8_t scancode = inb(0x60);
            switch (scancode) {
                case 0x2A: case 0x36: shift_pressed = 1; continue;
                case 0xAA: case 0xB6: shift_pressed = 0; continue;
            }
            if (scancode < 0x80) {
                const char* map = shift_pressed ? scancode_map_azerty_shifted : scancode_map_azerty_normal;
                if (scancode < sizeof(scancode_map_azerty_normal)) {
                    int key = map[scancode];
                    if (key != 0) return key;
                }
            }
        }
    }
}

// Version NON-BLOQUANTE (pour le jeu)
int check_key() {
    if (inb(0x64) & 0x1) {
        uint8_t scancode = inb(0x60);
        switch (scancode) {
            case 0x2A: case 0x36: shift_pressed = 1; return 0;
            case 0xAA: case 0xB6: shift_pressed = 0; return 0;
        }
        if (scancode < 0x80) {
            const char* map = shift_pressed ? scancode_map_azerty_shifted : scancode_map_azerty_normal;
            if (scancode < sizeof(scancode_map_azerty_normal)) {
                return map[scancode];
            }
        }
    }
    return 0;
}