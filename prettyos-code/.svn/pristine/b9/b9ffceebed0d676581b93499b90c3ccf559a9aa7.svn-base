#ifndef MOUSE_H
#define MOUSE_H

#include "util/util.h"
#include "video/video.h"

typedef enum
{
    BUTTON_LEFT = BIT(0),
    BUTTON_MIDDLE = BIT(1),
    BUTTON_RIGHT = BIT(2),
    BUTTON_4 = BIT(3),
    BUTTON_5 = BIT(4)
} mouse_button_t;


extern int32_t mouse_x;
extern int32_t mouse_y;
extern int32_t mouse_zv; // vertical mousewheel
extern int32_t mouse_zh; // horizontal mousewheel
extern mouse_button_t mouse_buttons; // Status of mouse buttons


void mouse_install(void);
void mouse_uninstall(void);
void mouse_setPosition(int32_t x, int32_t y);
void mouse_getPosition(position_t* pos);
bool mouse_buttonPressed(mouse_button_t button);

void mouse_buttonEvent(bool pressed, mouse_button_t button);
void mouse_moveEvent(int8_t x, int8_t y);
void mouse_wheelEvent(int8_t v, int8_t h);
void mouse_print(void);


#endif
