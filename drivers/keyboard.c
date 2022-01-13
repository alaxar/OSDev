/* bkerndev - Bran's Kernel Development Tutorial
*  By:   Brandon F. (friesenb@gmail.com)
*  Desc: Keyboard driver
*
*  Notes: No warranty expressed or implied. Use at own risk. */
#include "../include/system.h"
#include "../include/types.h"

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */

uint8_t isLeftShirtPressed = FALSE;
uint8_t isRightShiftPressed = FALSE;

/* Handles the keyboard interrupt */

void keyboard_handler(struct regs *r) {
    unsigned char kbdus[128] =
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
      '9', '0', '-', '=', '\b',	/* Backspace */
      '\t',			/* Tab */
      'q', 'w', 'e', 'r',	/* 19 */
      't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',		/* Enter key */
        0,			/* 29   - Control */
      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
    '\'', '`',   0,		/* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
      'm', ',', '.', '/',   0,					/* Right shift */
      '*',
        0,	/* Alt */
      ' ',	/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
      '-',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
      '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0,   0,   0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0,	/* All other keys are undefined */
    };
    unsigned char scancode;
    unsigned char chr;
    /* Read from the keyboard's data buffer */
    scancode = port_byte_in(0x60);

    // printf("> ", -1, -1, 0);
    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
        /* You can use this one to see if the user released the
        *  shift, alt, or control keys... */
       switch (scancode)
       {
       case 0xAA:   // left shift released
          isLeftShirtPressed = FALSE;
         break;
       case 0xb6:     // right left release
          isRightShiftPressed = FALSE;
          break;
      case 0x8e:    // backspace
            set_cursor(get_cursor() - 1);
            print_char(' ', -1, -1, 0);
            set_cursor(get_cursor() - 1);
            break;
       default:
         break;
       }
    }
    else
    {
      chr = kbdus[scancode];
      if(chr != 0) {
        // LEFT SHIFT
        switch (isLeftShirtPressed | isRightShiftPressed)
        {
        case TRUE:
          terminal_accept_command(chr - 32);
          break;
        default:
          terminal_accept_command(chr);
          break;
        }

      } else {
        switch(scancode) { 
          case 0x4B:
            set_cursor(get_cursor() - 1);
            break;
          case 0x2a:
            isLeftShirtPressed = TRUE;
            break;
          case 0x36:
            isRightShiftPressed = TRUE;
            break;
        }
      }
    }
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install()
{
    irq_install_handler(1, keyboard_handler);
}