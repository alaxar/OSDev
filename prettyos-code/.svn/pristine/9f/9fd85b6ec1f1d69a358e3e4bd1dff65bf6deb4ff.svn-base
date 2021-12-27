#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "os.h"


// Select keyboard layout
#define KEYMAP GER    // German keyboard
/// #define KEYMAP US     // US keyboard


typedef enum
{
    __KEY_INVALID,
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_ACC, KEY_MINUS, KEY_EQUAL, KEY_BACKSL, KEY_BACK, KEY_SPACE, KEY_CAPS, KEY_TAB, KEY_OSQBRA, KEY_CSQBRA,
    KEY_LSHIFT, KEY_LCTRL, KEY_LGUI, KEY_LALT, KEY_RSHIFT, KEY_RCTRL, KEY_RGUI, KEY_ALTGR, KEY_MENU, KEY_ENTER, KEY_ESC,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_PRINT, KEY_SCROLL, KEY_PAUSE, KEY_NUM, KEY_INS, KEY_DEL,
    KEY_HOME, KEY_PGUP, KEY_END, KEY_PGDWN, KEY_ARRU, KEY_ARRL, KEY_ARRD, KEY_ARRR,
    KEY_KPSLASH, KEY_KPMULT, KEY_KPMIN, KEY_KPPLUS, KEY_KPEN, KEY_KPDOT,
    KEY_KP0, KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP4, KEY_KP5, KEY_KP6, KEY_KP7, KEY_KP8, KEY_KP9,
    KEY_SEMI, KEY_APPOS, KEY_COMMA, KEY_DOT, KEY_SLASH,
    KEY_GER_ABRA, // German keyboard has one key more than the international one. TODO: Find a better name.
    __KEY_LAST
} KEY_t;


void keyboard_install(void);

void keyboard_keyPressedEvent(KEY_t key, bool make);

char getch(void);
bool keyPressed(KEY_t key);


#endif
