/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "gui.h"
#include "util/util.h"
#include "util/array.h"
#include "hid/mouse.h"
#include "hid/keyboard.h"
#include "videoutils.h"
#include "gui_window.h"
#include "gui_button.h"


// cursor
extern BMPHeader_t cursor_start;

extern array(window_t*) window_list;


void gui_start(void)
{
    windowManager_init();
    button_t button;
    button_create(&button, 250, 220, 80, 20, "close");

    window_create("Window 1", 10, 30, 340, 250, 0);
    window_create("Window 2", 400, 30, 340, 250, 0);
    window_create("Window 3", 10, 320, 340, 250, 0);
    window_create("Window 4", 400, 320, 340, 250, 0);

    while (!keyPressed(KEY_ESC))
    {
        if (mouse_buttons & BUTTON_LEFT)
        {
            video_drawString(video_currentMode->device, "left Mouse Button Pressed", 10, 2);

            if (mouse_x > button.x && mouse_x < (button.x + button.width) && mouse_y > button.y && mouse_y < (button.y + button.height))
            {
                window_destroy(1);
            }

            for (uint16_t i = 1; i < window_list.size; i++)
            {
                if (window_list.data[i])
                {
                    if (mouse_x > window_list.data[i]->CloseButton.x && mouse_x < (window_list.data[i]->CloseButton.x + window_list.data[i]->CloseButton.width) && mouse_y > button.y && mouse_y < (window_list.data[i]->CloseButton.y + window_list.data[i]->CloseButton.height))
                    {
                        window_destroy(i);
                    }
                    else if (mouse_x > window_list.data[i]->x && mouse_x < (window_list.data[i]->x + window_list.data[i]->width) && mouse_y >(window_list.data[i]->y) && mouse_y < (window_list.data[i]->y + 20))
                    {
                        window_list.data[i]->x = mouse_x;
                        window_list.data[i]->y = mouse_y;
                    }
                }
            }
        }

        for (uint16_t i = 1; i < window_list.size; i++)
        {
            if (window_list.data[i])
            {
                window_draw(i);
            }
        }

        if (window_list.data[1])
        {
            button_draw(video_currentMode->device, &button); // TODO: Associate Controls with a window. Draw all of them in the windows drawing function
        }

        video_drawString(video_currentMode->device, "Press ESC to Exit!", 10, 2);
        video_drawBitmapTransparent(video_currentMode->device, mouse_x, mouse_y, &cursor_start, white);
        video_flipScreen(video_currentMode->device);
    }

    button_destroy(&button);
    gui_end();
}

void gui_end(void)
{
    windowManager_end();
}

/*
* Copyright (c) 2010-2016 The PrettyOS Project. All rights reserved.
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
