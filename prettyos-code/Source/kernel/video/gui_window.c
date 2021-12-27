/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "gui_window.h"
#include "videoutils.h"
#include "kheap.h"
#include "util/util.h"
#include "util/array.h"


static const BGRA_t WINDOW_COLOUR_BACKGROUND = {191, 227, 197, 0};
static const BGRA_t WINDOW_COLOUR_BORDER = {2, 125, 57, 0};
static const BGRA_t WINDOW_COLOUR_TOPBAR = {253, 100, 100, 0};
static const BGRA_t WINDOW_COLOUR_FOCUS_TOPBAR = {127, 255, 0, 0};

static window_t* volatile current_window = 0;
array(window_t*) window_list;


void windowManager_init(void)
{
    window_t* desktop = malloc(sizeof(window_t), 0, "desktop window");
    // We need to initialise the Desktop
    desktop->name = "Desktop";
    desktop->x = 0;
    desktop->y = 0;
    desktop->z = 0;
    desktop->renderDevice = video_currentMode->device;
    desktop->width = video_currentMode->xRes;
    desktop->height = video_currentMode->yRes;
    desktop->parentid = 0;
    desktop->id = HWND_DESKTOP;

    array_construct(&window_list);
    array_resize(&window_list, 1);
    window_list.data[desktop->id] = desktop;

    current_window = desktop;
}

void windowManager_end(void)
{
    current_window = 0;
    for (uint16_t i = 1; i < window_list.size; i++)
    {
        if (window_list.data[i] != 0)
            window_destroy(i);
    }
    array_destruct(&window_list);
}

static uint16_t getnewwid(void)
{
    static uint16_t wid = 0;
    wid++;
    return wid;
}

void window_destroy(uint16_t id)
{
    button_destroy(&window_list.data[id]->CloseButton);
    if (id != HWND_DESKTOP)
        video_freeDevice(window_list.data[id]->renderDevice);
    free(window_list.data[id]);
    window_list.data[id] = 0;
}

void window_create(char* windowname, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t parentid)
{
    window_t* window = malloc(sizeof(window_t), 0, "window");
    window->name = windowname;
    window->x = x;
    window->y = y;
    window->z = 1;
    window->width = width;
    window->height = height;
    window->renderDevice = renderBuffer_create(width+4, height+24, 32)->device;
    window->parentid = parentid;
    window->id = getnewwid();

    video_clearScreen(window->renderDevice, WINDOW_COLOUR_BACKGROUND);

    button_create(&window->CloseButton, window->width - 17, 2, 17, 17, "X");

    // And set window focus
    current_window = window;

    array_resize(&window_list, max(window_list.size, window->id+1));
    window_list.data[window->id] = window;
}

void window_draw(uint16_t id)
{
    window_t* window = window_list.data[id];
    // Border
    video_drawRect(window->renderDevice, 1, 1, window->renderDevice->videoMode.xRes-2, window->renderDevice->videoMode.yRes-2, WINDOW_COLOUR_BORDER);
    video_drawRect(window->renderDevice, 0, 0, window->renderDevice->videoMode.xRes-1, window->renderDevice->videoMode.yRes-1, WINDOW_COLOUR_BORDER);

    // Topbar
    if (window == current_window)
        video_drawRectFilled(window->renderDevice, 2, 2, window->renderDevice->videoMode.xRes - 3, 20, WINDOW_COLOUR_FOCUS_TOPBAR);
    else
        video_drawRectFilled(window->renderDevice, 2, 2, window->renderDevice->videoMode.xRes - 3, 20, WINDOW_COLOUR_TOPBAR);

    // Title
    video_drawString(window->renderDevice, window_list.data[id]->name, 3, 3);

    // Data
    button_draw(window->renderDevice, &window_list.data[id]->CloseButton);

    renderBuffer_render(video_currentMode->device, window->renderDevice->data, window->x, window->y); // Check ->data
}

/*
* Copyright (c) 2010-2015 The PrettyOS Project. All rights reserved.
*
* http://www.c-plusplus.de/forum/viewforum-var-f-is-62.html
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
