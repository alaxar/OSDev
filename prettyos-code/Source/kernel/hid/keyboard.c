/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "keyboard.h"
#include "util/util.h"
#include "tasking/task.h"
#include "irq.h"
#include "storage/devicemanager.h"

#if KEYMAP == GER
  #include "keyboard_GER.h"
#else //US-Keyboard if nothing else is defined
  #include "keyboard_US.h"
#endif


static const KEY_t scancodeToKey_default[] = // cf. http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html#ss1.4
{
//  0           1           2           3           4           5            6             7
//---------------------------------------------------------------------------------------------------------
    0,          KEY_ESC,    KEY_1,      KEY_2,      KEY_3,      KEY_4,       KEY_5,        KEY_6,      // 0
    KEY_7,      KEY_8,      KEY_9,      KEY_0,      KEY_MINUS,  KEY_EQUAL,   KEY_BACK,     KEY_TAB,
    KEY_Q,      KEY_W,      KEY_E,      KEY_R,      KEY_T,      KEY_Y,       KEY_U,        KEY_I,      // 1
    KEY_O,      KEY_P,      KEY_OSQBRA, KEY_CSQBRA, KEY_ENTER,  KEY_LCTRL,   KEY_A,        KEY_S,
    KEY_D,      KEY_F,      KEY_G,      KEY_H,      KEY_J,      KEY_K,       KEY_L,        KEY_SEMI,   // 2
    KEY_APPOS,  KEY_ACC,    KEY_LSHIFT, KEY_BACKSL, KEY_Z,      KEY_X,       KEY_C,        KEY_V,
    KEY_B,      KEY_N,      KEY_M,      KEY_COMMA,  KEY_DOT,    KEY_SLASH,   KEY_RSHIFT,   KEY_KPMULT, // 3
    KEY_LALT,   KEY_SPACE,  KEY_CAPS,   KEY_F1,     KEY_F2,     KEY_F3,      KEY_F4,       KEY_F5,
    KEY_F6,     KEY_F7,     KEY_F8,     KEY_F9,     KEY_F10,    KEY_NUM,     KEY_SCROLL,   KEY_KP7,    // 4
    KEY_KP8,    KEY_KP9,    KEY_KPMIN,  KEY_KP4,    KEY_KP5,    KEY_KP6,     KEY_KPPLUS,   KEY_KP1,
    KEY_KP2,    KEY_KP3,    KEY_KP0,    KEY_KPDOT,  0,          0,           KEY_GER_ABRA, KEY_F11,    // 5
    KEY_F12,    0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 6
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 7
    0,          0,          0,          0,          0,          0,           0,            0,
};

static const KEY_t scancodeToKey_E0[] = // cf. http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html#ss1.5
{
//  0           1           2           3           4           5            6             7
//---------------------------------------------------------------------------------------------------------
    0,          0,          0,          0,          0,          0,           0,            0,          // 0
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 1
    0,          0,          0,          0,          KEY_KPEN,   KEY_RCTRL,   0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 2
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          KEY_KPSLASH, 0,            KEY_PRINT,  // 3
    KEY_ALTGR,  0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            KEY_HOME,   // 4
    KEY_ARRU,   KEY_PGUP,   0,          KEY_ARRL,   0,          KEY_ARRR,    0,            KEY_END,
    KEY_ARRD,   KEY_PGDWN,  KEY_INS,    KEY_DEL,    0,          0,           0,            0,          // 5
    0,          0,          0,          KEY_LGUI,   KEY_RGUI,   0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 6
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 7
    0,          0,          0,          0,          0,          0,           0,            0,
};

static bool pressedKeys[__KEY_LAST] = {false}; // for monitoring pressed keys
static void keyboard_handler(registers_t* r);
static uint8_t LED = 0x00;

static void keyboard_updateLED(void)
{
    outportb(0x60, 0xED);
    inportb(0x60);
    outportb(0x60, LED);
    inportb(0x60);
}

void keyboard_install(void)
{
    irq_installHandler(IRQ_KEYBOARD, keyboard_handler); // Installs 'keyboard_handler' to IRQ_KEYBOARD

    while (inportb(0x64) & 1) // wait until buffer is empty
    {
        inportb(0x60);
    }

    keyboard_updateLED();
}

static uint8_t getScancode(void)
{
    uint8_t scancode = 0;

    if (inportb(0x64)&1)
        scancode = inportb(0x60);   // 0x60: get scan code from the keyboard

    // ACK: toggle bit 7 at port 0x61
    uint8_t port_value = inportb(0x61);
    outportb(0x61, port_value |  0x80); // 0->1
    outportb(0x61, port_value &~ 0x80); // 1->0

    return (scancode);
}

static KEY_t scancodeToKey(uint8_t scancode, bool* make)
{
    KEY_t key = __KEY_INVALID;
    static uint8_t prevScancode = 0; // Stores the previous scancode. For E1 codes it stores always the first byte (0xE1).
    static uint8_t byteCounter = 1; // Only needed for E1 codes

    *make = !(scancode & 0x80); // make code

    if (scancode == 0xE0) // First byte of E0 code
    {
        prevScancode = 0xE0;
    }
    else if (scancode == 0xE1) // First byte of E1 code
    {
        prevScancode = 0xE1;
        byteCounter  = 1;
    }
    else
    {
        if (prevScancode == 0xE0) // Second byte of E0 code
        {
            prevScancode = 0; // Last scancode is not interesting in this case
            key = scancodeToKey_E0[scancode & 0x7F];
        }
        else if (prevScancode == 0xE1) // Second or third byte of E1 code. HACK: We assume, that all E1 codes mean the pause key
        {
            byteCounter++;
            if (byteCounter == 3)
            {
                prevScancode = 0; // Last scancode is not interesting in this case
                return (KEY_PAUSE);
            }
        }
        else // Default code
        {
            prevScancode = 0; // Last scancode is not interesting in this case
            key = scancodeToKey_default[scancode & 0x7F];
        }
    }
    return (key);
}

static char keyToASCII(KEY_t key)
{
    char retchar = 0; // The character that returns the scan code to ASCII code

    bool shift = pressedKeys[KEY_LSHIFT] || pressedKeys[KEY_RSHIFT];
    if (LED & BIT(2)) // Caps-Lock
        shift = !shift;

    // Fallback mechanism
    if (pressedKeys[KEY_ALTGR])
    {
        if (shift)
        {
            retchar = keyToASCII_shiftAltGr[key];
        }
        if (!shift || retchar == 0) // if shift is not pressed or if there is no key specified for ShiftAltGr (so retchar is still 0)
        {
            retchar = keyToASCII_altGr[key];
        }
    }
    if (!pressedKeys[KEY_ALTGR] || retchar == 0) // if AltGr is not pressed or if retchar is still 0
    {
        if (shift)
        {
            retchar = keyToASCII_shift[key];
        }
        if (!shift || retchar == 0) // if shift is not pressed or if retchar is still 0
        {
            retchar = keyToASCII_default[key];
        }
    }

    // filter special key combinations
    if (pressedKeys[KEY_LALT]) // Console-Switching
    {
        if (retchar == 'm')
        {
            console_display(KERNELCONSOLE_ID);
            return (0);
        }
        if (key == KEY_PGUP)
        {
            console_incrementStack();
            return (0);
        }
        if (key == KEY_PGDWN)
        {
            console_decrementStack();
            return (0);
        }
        if (retchar == 'f')
        {
            console_foldStack();
            return (0);
        }
        if (ctoi(retchar) != -1)
        {
            console_display(ctoi(retchar));
            return (0);
        }
    }
    if (pressedKeys[KEY_RCTRL] || pressedKeys[KEY_LCTRL])
    {
        if (key == KEY_ESC || retchar == 'e')
        {
            list_t* list = &console_displayed->tasks;
            for (dlelement_t* e = list->head; e != 0;)
            {
                task_t* task = e->data;

                if (task->pid != 0)
                {
                    kill(task);
                    e = list->head; // Restart at beginning, because list has been modified by kill()
                }
                else
                {
                    e = e->next;
                }
            }

            return (0);
        }
    }

    static bool PRINT;
    if (key == KEY_PRINT || key == KEY_F12) // Save content of video memory. F12 is alias for PrintScreen due to problems in some emulators
    {
        takeScreenshot();
        PRINT = true;
    }
    else if (PRINT)
    {
        PRINT = false;
        switch (retchar)
        {
        case 'f': // Taking a screenshot (Floppy)
            printf("Save screenshot to Floppy.");
            saveScreenshot(&FLOPPYDISK);
            break;
        case 'u': // Taking a screenshot (USB)
            printf("Save screenshot to USB.");
            saveScreenshot(&USB_MSD);
            break;
#ifdef _ENABLE_HDD_
        case 'h': // Taking a screenshot (HDD)
            printf("Save screenshot to HDD.");
            saveScreenshot(&HDDDISK);
            break;
#endif
        }
    }

    return (retchar);
}

static void keyboard_handler(registers_t* r)
{
    while (inportb(0x64)&1)
    {
        // Get scancode
        uint8_t scancode = getScancode();
        bool make = false;

        // Find out key
        KEY_t key = scancodeToKey(scancode, &make);

        // Issue Events
        keyboard_keyPressedEvent(key, make);
    }
}

void keyboard_keyPressedEvent(KEY_t key, bool make)
{
    if (key == __KEY_INVALID)
    {
        return;
    }

    pressedKeys[key] = make;

    if (make)
    {
        if (key == KEY_CAPS)
        {
            LED ^= BIT(2);
            keyboard_updateLED();
        }
        else if(key == KEY_NUM)
        {
            LED ^= BIT(1);
            keyboard_updateLED();
        }
        else if (key == KEY_SCROLL)
        {
            LED ^= BIT(0);
            keyboard_updateLED();
        }

        event_issueToDisplayedTasks(EVENT_KEY_DOWN, &key, sizeof(KEY_t));
    }
    else
    {
        event_issueToDisplayedTasks(EVENT_KEY_UP, &key, sizeof(KEY_t));
        return;
    }

    // Find out ASCII representation of key
    char ascii = keyToASCII(key);
    if (ascii)
        event_issueToDisplayedTasks(EVENT_TEXT_ENTERED, &ascii, sizeof(char));
}

char getch(void)
{
    char ret = 0;
    EVENT_t ev = event_poll(&ret, 1, EVENT_NONE);

    while (ev != EVENT_TEXT_ENTERED)
    {
        if (ev == EVENT_NONE)
        {
            waitForEvent(0);
        }

        ev = event_poll(&ret, 1, EVENT_NONE);
    }
    return (ret);
}

bool keyPressed(KEY_t key)
{
    return (pressedKeys[key]);
}

/*
* Copyright (c) 2009-2017 The PrettyOS Project. All rights reserved.
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
