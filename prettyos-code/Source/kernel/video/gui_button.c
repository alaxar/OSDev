/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "gui_button.h"
#include "videoutils.h"
#include "kheap.h"
#include "util/util.h"


static const BGRA_t BUTTON_COLOUR = {128, 128, 128, 0};
static const BGRA_t BUTTON_COLOUR_BORDER = {195, 195, 195, 0};

void button_create(button_t* button, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const char* label)
{
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->label = strdup(label, "button->label");
}

void button_draw(videoDevice_t* device, button_t* button)
{
    video_drawRectFilled(device, button->x, button->y, button->x+button->width, button->y+button->height, BUTTON_COLOUR);
    video_drawRect(device, button->x, button->y, button->x+button->width, button->y+button->height, BUTTON_COLOUR_BORDER);
    video_drawString(device, button->label, button->x - 4*strlen(button->label) + button->width/2, button->y - 7 + button->height/2);
}

void button_destroy(button_t* button)
{
    free(button->label);
}

/*
* Copyright (c) 2010-2015 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
