#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "videomanager.h"


typedef struct
{
    char* label;
    void* data;
    uint16_t id;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} button_t;

void button_create(button_t* button, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char* label);
void button_destroy(button_t* button);
void button_draw(videoDevice_t* device, button_t* button);

#endif
