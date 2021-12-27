/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "usb_hid.h"
#include "usb/usb_hc.h"
#include "kheap.h"
#include "video/console.h"
#include "mouse.h"
#include "keyboard.h"


static const KEY_t usageIdToKey[] =
{
//  0           1           2           3           4           5            6             7
//---------------------------------------------------------------------------------------------------------
    0,          0,          0,          0,          KEY_A,      KEY_B,       KEY_C,        KEY_D,      // 0
    KEY_E,      KEY_F,      KEY_G,      KEY_H,      KEY_I,      KEY_J,       KEY_K,        KEY_L,
    KEY_M,      KEY_N,      KEY_O,      KEY_P,      KEY_Q,      KEY_R,       KEY_S,        KEY_T,      // 1
    KEY_U,      KEY_V,      KEY_W,      KEY_X,      KEY_Y,      KEY_Z,       KEY_1,        KEY_2,
    KEY_3,      KEY_4,      KEY_5,      KEY_6,      KEY_7,      KEY_8,       KEY_9,        KEY_0,      // 2
    KEY_ENTER,  KEY_ESC,    KEY_BACK,   KEY_TAB,    KEY_SPACE,  KEY_MINUS,   KEY_EQUAL,    KEY_OSQBRA,
    KEY_CSQBRA, KEY_BACKSL, 0,          KEY_SEMI,   0,          KEY_ACC,     KEY_COMMA,    KEY_DOT,    // 3
    KEY_SLASH,  KEY_CAPS,   KEY_F1,     KEY_F2,     KEY_F3,     KEY_F4,      KEY_F5,       KEY_F6,
    KEY_F7,     KEY_F8,     KEY_F9,     KEY_F10,    KEY_F11,    KEY_F12,     KEY_PRINT,    KEY_SCROLL, // 4
    KEY_PAUSE,  KEY_INS,    KEY_HOME,   KEY_PGUP,   KEY_DEL,    KEY_END,     KEY_PGDWN,    KEY_ARRR,
    KEY_ARRL,   KEY_ARRD,   KEY_ARRU,   KEY_NUM,    KEY_KPSLASH,KEY_KPMULT,  KEY_KPMIN,    KEY_KPPLUS, // 5
    KEY_KPEN,   KEY_KP1,    KEY_KP2,    KEY_KP3,    KEY_KP4,    KEY_KP5,     KEY_KP6,      KEY_KP7,
    KEY_KP8,    KEY_KP9,    KEY_KP0,    KEY_KPDOT,  0,          0,           0,            0,          // 6
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 7
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 8
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 9
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 10
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 11
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 12
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 13
    0,          0,          0,          0,          0,          0,           0,            0,
    KEY_LCTRL,  KEY_LSHIFT, KEY_LALT,   KEY_LGUI,   KEY_RCTRL,  KEY_RSHIFT,  KEY_ALTGR,    KEY_RGUI,   // 14
    0,          0,          0,          0,          0,          0,           0,            0,
    0,          0,          0,          0,          0,          0,           0,            0,          // 15
    0,          0,          0,          0,          0,          0,           0,            0
};

static bool usb_getHidDescriptor(usb_device_t* device, uint8_t descType, uint8_t descIndex, uint16_t interfaceNum);
static void parseMouseReport(usb_transfer_t* transfer, void* data);
static void parseKeyboardReport(usb_transfer_t* transfer, void* data);


void usb_setupHID(usb_interface_t* interface)
{
    textColor(HEADLINE);
    printf("\n\nSetup HID...");
    textColor(TEXT);

    usb_hid_t* hid = malloc(sizeof(usb_hid_t), 0, "usb_hid_t");
    interface->data = hid;
    hid->interface = interface;
    hid->buffer = 0;
    hid->LEDs = 0;

    // Get interrupt endpoint (IN)
    usb_endpoint_t* endpointInterrupt = 0;
    for (dlelement_t* el = interface->device->endpoints.head; el; el = el->next)
    {
        usb_endpoint_t* ep = el->data;
        if (ep->type == EP_INTERRUPT && ep->dir == EP_IN)
        {
            endpointInterrupt = ep;
            break;
        }
    }
    if (!endpointInterrupt)
    {
        printfe("\nHID has no interrupt endpoint!");
        return;
    }

    bool success = usb_getHidDescriptor(interface->device, DESC_HID, 0, interface->descriptor.interfaceNumber);
    if (!success)
    {
        printfe("\nHID-Descriptor could not be read!");
        return;
    }

    printf("\nInterrupt endpoint: mps=%u, interval=%u", endpointInterrupt->mps, endpointInterrupt->interval);
    printf("\nInterface protocol: ");
    textColor(DATA);
    switch (interface->descriptor.interfaceProtocol)
    {
        case 1:
            puts("Keyboard ");
            if (interface->descriptor.interfaceSubclass == 1)
            {
                puts("(boot)");
                hid->buffer = malloc(8, 0, "usb keyboard buffer"); // Keyboard sends reports of eight bytes
                usb_startInterruptInTransfer(&hid->interruptTransfer, hid->interface->device, endpointInterrupt, hid->buffer, 8, &parseKeyboardReport, hid, endpointInterrupt->interval);
            }
            break;
        case 2:
            puts("Mouse ");
            if (interface->descriptor.interfaceSubclass == 1)
            {
                puts("(boot)");
                size_t size = clamp(3, endpointInterrupt->mps, 8); // Mouse sends reports of three or more bytes. We are only interested in the first three.
                hid->buffer = malloc(size, 0, "usb mouse buffer");
                usb_startInterruptInTransfer(&hid->interruptTransfer, hid->interface->device, endpointInterrupt, hid->buffer, size, &parseMouseReport, hid, endpointInterrupt->interval);
            }
            break;
        case 0: default:
            printf("None (%u)", interface->descriptor.interfaceProtocol);
            break;
    }
    textColor(TEXT);
}

void usb_destroyHID(usb_interface_t* interface)
{
    usb_hid_t* hid = interface->data;
    if (hid->buffer)
    {
        usb_destructTransfer(&hid->interruptTransfer);
        free(hid->buffer);
    }
    free(hid);
}

static void parseMouseReport(usb_transfer_t* transfer, void* data)
{
    usb_hid_t* hid = data;
    mouse_moveEvent(hid->buffer[1], hid->buffer[2]);
    mouse_buttonEvent(hid->buffer[0] & BIT(0), BUTTON_LEFT);
    mouse_buttonEvent(hid->buffer[0] & BIT(1), BUTTON_RIGHT);
    mouse_buttonEvent(hid->buffer[0] & BIT(2), BUTTON_MIDDLE);
    mouse_print();
}

static void parseFlag(uint8_t flag, uint8_t bit, KEY_t key)
{
    bool pressed = (flag & BIT(bit)) != 0;
    if (pressed != keyPressed(key))
    {
        keyboard_keyPressedEvent(key, pressed);
    }
}

static void parseKeyboardReport(usb_transfer_t* transfer, void* data)
{
    usb_hid_t* hid = data;

    // The first byte of the report consists of 8 flags indicating the status of the Ctrl, Shift, Alt and GUI Keys
    parseFlag(hid->buffer[0], 0, KEY_LCTRL);
    parseFlag(hid->buffer[0], 1, KEY_LSHIFT);
    parseFlag(hid->buffer[0], 2, KEY_LALT);
    parseFlag(hid->buffer[0], 3, KEY_LGUI);
    parseFlag(hid->buffer[0], 4, KEY_RCTRL);
    parseFlag(hid->buffer[0], 5, KEY_RSHIFT);
    parseFlag(hid->buffer[0], 6, KEY_ALTGR);
    parseFlag(hid->buffer[0], 7, KEY_RGUI);

    // There are no notifications when keys are released, instead we have to check all pressed keys if they are still reported as pressed.
    for (KEY_t key = 0; key < __KEY_LAST; key++)
    {
        if (key == KEY_LCTRL || key == KEY_LSHIFT || key == KEY_LALT || key == KEY_LGUI ||
            key == KEY_RCTRL || key == KEY_RSHIFT || key == KEY_ALTGR || key == KEY_RGUI)
            continue;

        if (keyPressed(key))
        {
            bool stillPressed = false;
            for (int i = 0; i < 6; i++)
            {
                KEY_t pressedKey = usageIdToKey[hid->buffer[2 + i]];
                if (pressedKey == key)
                {
                    stillPressed = true;
                    hid->buffer[2 + i] = 0; // Avoid spurious repetitions of keyPressedEvent in the code below
                    break;
                }
            }
            if (!stillPressed)
            {
                keyboard_keyPressedEvent(key, false);
            }
        }
    }

    // USB Boot Keyboards support up to 6-Key-Rollover (in contrast to PS/2, which supports up to N-Key-Rollover)
    for (uint8_t i = 0; i < 6; i++)
    {
        KEY_t key = usageIdToKey[hid->buffer[2 + i]];
        keyboard_keyPressedEvent(key, true);
        if (key == KEY_CAPS || key == KEY_NUM || key == KEY_SCROLL)
        {
            if (key == KEY_CAPS)
                hid->LEDs ^= BIT(1);
            else if (key == KEY_NUM)
                hid->LEDs ^= BIT(0);
            else if (key == KEY_SCROLL)
                hid->LEDs ^= BIT(2);
            usb_controlOut(hid->interface->device, &hid->LEDs, 0x21, HID_SET_REPORT, 2, 0, hid->interface->descriptor.interfaceNumber, 1);
        }
    }
}

#ifdef _USB_HID_DIAGNOSIS_
static void showHidDescriptor(usb_hidDescriptor_t* d)
{
    textColor(IMPORTANT);
    printf("\nHID descriptor (size: %u):", d->length);
    textColor(TEXT);

    printf("\n    HID descriptor type:    ");
    switch (d->descType)
    {
        case DESC_HID:      printf("HID Descriptor"); break;
        case DESC_REPORT:   printf("Report Descriptor"); break;
        case DESC_PHYSICAL: printf("Physical Descriptor"); break;
    }

    printf("\n    Specification release:  %x", d->cdHID);
    printf("\n    Country code:           %u", d->countryCode);
    printf("\n    Number of class desc:   %u", d->numDescriptors);

    printf("\n    Descriptor type 2:      ");
    switch (d->descType2)
    {
        case DESC_HID:      printf("HID Descriptor"); break;
        case DESC_REPORT:   printf("Report Descriptor"); break;
        case DESC_PHYSICAL: printf("Physical Descriptor"); break;
    }

    printf(" (size: %u)\n", d->descLength);
}
#endif

static bool usb_getHidDescriptor(usb_device_t* device, uint8_t descType, uint8_t descIndex, uint16_t interfaceNum)
{
  #ifdef _USB_TRANSFER_DIAGNOSIS_
    textColor(LIGHT_CYAN);
    printf("\n\nUSB: GET_DESCRIPTOR HID");
    textColor(TEXT);
  #endif

    usb_hidDescriptor_t descriptor;

    bool success = usb_controlIn(device, &descriptor, 0x81, 6, descType, descIndex, interfaceNum, sizeof(descriptor));

    if (success)
    {
      #ifdef _USB_HID_DIAGNOSIS_
        showHidDescriptor(&descriptor);
      #endif
    }

    return success;
}

/*
* Copyright (c) 2015-2017 The PrettyOS Project. All rights reserved.
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
