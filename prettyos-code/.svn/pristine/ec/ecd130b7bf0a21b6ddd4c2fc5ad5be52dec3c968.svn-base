/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss f?r die Verwendung dieses Sourcecodes siehe unten
*/

#include "console.h"
#include "util/util.h"
#include "kheap.h"
#include "tasking/task.h"
#include "network/netutils.h"


console_t kernelConsole = // The console of the kernel task. It is a global variable because it should be initialized as soon as possible.
{
    .stack = 0, .ID = KERNELCONSOLE_ID, .name = 0, .properties = CONSOLE_AUTOSCROLL|CONSOLE_AUTOREFRESH|CONSOLE_SHOWINFOBAR,
    .scrollBegin = 0, .scrollEnd = 39, .cursor = {0, 0}, .mutex = mutex_init(), .tasks = list_init()
};

static list_t consoleStacks = list_init(); // Contains arrays of each 10 consoles
dlelement_t* console_currentStack = 0;
console_t*          console_current   = &kernelConsole; // The console of the active task
console_t* volatile console_displayed = &kernelConsole; // Currently visible console

static bool scroll_flag = true;


static void scroll(void);
static void displayConsole(console_t* console);


uint8_t getTextColor(void)
{
    return (currentTask->attrib);
}

void textColor(uint8_t color) // bit 0-3: foreground bit 4-7: background
{
    if (currentTask)
    {
        currentTask->attrib = color;
    }
}

void kernel_console_init(void)
{
    memsetw(kernelConsole.vidmem, 0, COLUMNS * LINES);
}

void console_init(console_t* console, const char* name)
{
    console->name        = strdup(name, "console-name");
    console->cursor.x    = 0;
    console->cursor.y    = 0;
    console->scrollBegin = 0;
    console->scrollEnd   = USER_END-USER_BEGIN;
    console->properties  = CONSOLE_AUTOREFRESH|CONSOLE_AUTOSCROLL;
    mutex_construct(&console->mutex);
    list_construct(&console->tasks);
    memsetw(console->vidmem, 0, COLUMNS * LINES);

    for (dlelement_t* e = consoleStacks.head; e; e = e->next)
    {
        consoleStack_t* consoleStack = e->data;
        for (uint8_t i = 0; i < 10; i++)
        {
            if (consoleStack->array[i] == 0)
            {
                consoleStack->array[i] = console;
                console->ID = i;
                console->stack = e;
                displayConsole(console); // Switching to the new console
                return;
            }
        }
    }
    // No free stack - add a new one
    dlelement_t* elem = list_alloc_elem(sizeof(consoleStack_t), "console stack");
    consoleStack_t* consoleStack = elem->data;
    memset(consoleStack->array, 0, 10 * sizeof(console_t*));
    consoleStack->base = 0;
    if (consoleStacks.tail)
        consoleStack->base = ((consoleStack_t*)consoleStacks.tail->data)->base + 10;
    consoleStack->array[0] = console;
    console->ID = 0;
    console->stack = elem;
    list_append_elem(&consoleStacks, elem);
    displayConsole(console); // Switching to the new console
}

void console_exit(console_t* console)
{
    if (console->stack)
    {
        consoleStack_t* consoleStack = console->stack->data;
        consoleStack->array[console->ID] = 0;
        bool empty = true;
        for (uint8_t i = 0; i < 10; i++)
        {
            if (consoleStack->array[i] != 0)
            {
                empty = false;
                break;
            }
        }
        if (empty)
        {
            if (console->stack == console_currentStack)
            {
                console_currentStack = consoleStacks.head;
                if (console->stack == console_currentStack)
                    console_currentStack = 0;
            }
            list_delete(&consoleStacks, console->stack);
        }
    }

    // Switch back to kernel console, if the tasks console is displayed at the moment
    if (console == console_displayed)
        console_display(KERNELCONSOLE_ID);

    free(console->name);
    list_destruct(&console->tasks);
    mutex_destruct(&console->mutex);
}

void console_cleanup(task_t* task)
{
    mutex_lock(&task->console->mutex);

    list_delete(&task->console->tasks, list_find(&task->console->tasks, task));
    if (task->console->tasks.head == 0)
    {
        // Free memory
        console_exit(task->console);
        free(task->console);
    }

    mutex_unlock(&task->console->mutex);
}

static void displayConsole(console_t* console)
{
    if (console == console_displayed)
        return;
    event_issueToDisplayedTasks(EVENT_CONSOLE_LOST_FOCUS, 0, 0);
    if (console->stack)
        console_currentStack = console->stack;
    console_displayed = console;

    if (console_displayed->properties & CONSOLE_AUTOREFRESH)
        refreshScreen();

    vga_updateCursor();
    event_issueToDisplayedTasks(EVENT_CONSOLE_GAINED_FOCUS, 0, 0);
}

bool console_display(uint8_t ID)
{
    if (ID == 11) // 11 is kernel console, regardless of stack
    {
        displayConsole(&kernelConsole);
        return (true);
    }

    // Changing visible console, returning false, if this console is not available.
    if (ID > 10 || !consoleStacks.head)
        return (false);
    if (!console_currentStack)
        console_currentStack = consoleStacks.head;
    consoleStack_t* consoleStack = console_currentStack->data;
    if (!consoleStack->array[ID])
        return (false);

    displayConsole(consoleStack->array[ID]);
    return (true);
}

static void gotoFirstConsole(void)
{
    for (uint8_t i = 0; i < 10; i++)
    {
        if (console_display(i))
            return;
    }
}

void console_incrementStack(void)
{
    if (console_currentStack)
    {
        dlelement_t* oldStack = console_currentStack;
        console_currentStack = console_currentStack->next;
        if (console_currentStack == 0) // Overflow
            console_currentStack = consoleStacks.head;
        if (console_currentStack != oldStack)
            gotoFirstConsole();
    }
}

void console_decrementStack(void)
{
    if (console_currentStack)
    {
        dlelement_t* oldStack = console_currentStack;
        console_currentStack = console_currentStack->prev;
        if (console_currentStack == 0) // Underflow
            console_currentStack = consoleStacks.tail;
        if (console_currentStack != oldStack)
            gotoFirstConsole();
    }
}

void console_foldStack(void)
{
    if (!consoleStacks.head)
        return;
    dlelement_t* new = consoleStacks.head;
    dlelement_t* old = consoleStacks.head;
    uint8_t newid = 0;
    consoleStack_t* newStack = new->data;
    newStack->base = 0;
    while (old) // Iterate over old arrangement
    {
        consoleStack_t* oldStack = old->data;
        for (uint8_t oldid = 0; oldid < 10; oldid++)
        {
            if (oldStack->array[oldid])   // Element in old arrangement
            {
                consoleStack_t* temp = new->data;
                temp->base = newStack->base;
                newStack = temp;
                newStack->array[newid] = oldStack->array[oldid];
                newStack->array[newid]->stack = new;
                newStack->array[newid]->ID = newid;
                newid++;
                if (newid > 10)
                {
                    newid = 0;
                    new = new->next;
                }
            }
        }
        old = old->next;
    }
    memset(newStack->array + newid, 0, sizeof(console_t*)*(10 - newid));
    while (new->next)
        list_delete(&consoleStacks, new->next);
}

void setScrollField(uint8_t begin, uint8_t end)
{
    console_current->scrollBegin = begin;
    console_current->scrollEnd = end;
}

void console_setProperties(console_properties_t properties)
{
    console_current->properties = properties;

    if(properties & CONSOLE_SHOWINFOBAR)
        console_current->scrollEnd = min(console_current->scrollEnd, 42);
    if(properties & CONSOLE_AUTOREFRESH)
        refreshScreen();
}

void console_clear(uint8_t backcolor)
{
    mutex_lock(&console_current->mutex);
    // Erasing the content of the active console
    memsetw(console_current->vidmem, 0x20U | (backcolor << 8), COLUMNS * LINES);
    console_current->cursor.x = 0;
    console_current->cursor.y = 0;

    if (console_current == console_displayed && (console_current->properties & CONSOLE_AUTOREFRESH)) // If it is also displayed at the moment, refresh screen
    {
        refreshUserScreen();
        vga_updateCursor();
    }

    mutex_unlock(&console_current->mutex);
}

static void move_cursor_right(void)
{
    ++console_current->cursor.x;

    if (console_current->cursor.x >= COLUMNS)
    {
        ++console_current->cursor.y;
        console_current->cursor.x = 0;
        scroll();
    }
}

static void move_cursor_left(void)
{
    if (console_current->cursor.x)
    {
        --console_current->cursor.x;
    }
    else if (console_current->cursor.y > 0)
    {
        console_current->cursor.x = COLUMNS-1;
        --console_current->cursor.y;
    }
}

static void move_cursor_home(void)
{
    console_current->cursor.x = 0;
}

void setCursor(position_t pos)
{
    pos.x = min(COLUMNS-1, pos.x);
    pos.y = min(LINES-1, pos.y);
    console_current->cursor = pos;

    if (console_current == console_displayed)
        vga_updateCursor();
}

void getCursor(position_t* pos)
{
    *pos = console_current->cursor;
}

void console_setPixel(uint8_t x, uint8_t y, uint16_t value)
{
    mutex_lock(&console_current->mutex);
    console_current->vidmem[y*COLUMNS + x] = value;
    mutex_unlock(&console_current->mutex);

    if (console_current == console_displayed && (console_current->properties & CONSOLE_AUTOREFRESH))
    {
        vga_setPixel(x, y+2, value);
    }
}

static void putCP437ch(uint8_t uc)
{
    switch (uc)
    {
        case 0x08: // backspace: move the cursor one space backwards and delete
            move_cursor_left();
            *(console_current->vidmem + console_current->cursor.y * COLUMNS + console_current->cursor.x) = (uint16_t)' ' | (uint16_t)(getTextColor() << 8);
            if (console_displayed == console_current && (console_current->properties & CONSOLE_AUTOREFRESH)) // Print to screen, if current console is displayed at the moment
                vga_setPixel(console_current->cursor.x, console_current->cursor.y + 2, (uint16_t)' ' | (uint16_t)(getTextColor() << 8));
            break;
        case 0x09: // tab: increment cursor.x (divisible by 8)
            console_current->cursor.x = alignUp(console_current->cursor.x+1, 8);
            if (console_current->cursor.x>=COLUMNS)
            {
                ++console_current->cursor.y;
                console_current->cursor.x=0;
                scroll();
            }
            break;
        case '\r': // cr: cursor back to the margin
            move_cursor_home();
            break;
        case '\n': // newline: like 'cr': cursor to the margin and increment cursor.y
            ++console_current->cursor.y;
            move_cursor_home();
            scroll();
            break;
        default:
        {
            uint16_t att = getTextColor() << 8;
            *(console_current->vidmem + console_current->cursor.y * COLUMNS + console_current->cursor.x) = uc | att; // character AND attributes: color
            if (console_displayed == console_current && (console_current->properties & CONSOLE_AUTOREFRESH)) // Print to screen, if current console is displayed at the moment
                vga_setPixel(console_current->cursor.x, console_current->cursor.y+2, uc | att); // character AND attributes: color
            move_cursor_right();
            break;
        }
    }
}

static void atomic_putch(char c) // Does neither lock mutex nor move vga cursor
{
    uint8_t uc = AsciiToCP437((uint8_t)c); // no negative values
    putCP437ch(uc);
}

void putch(char c)
{
    uint8_t uc = AsciiToCP437((uint8_t)c); // no negative values
    mutex_lock(&console_current->mutex);
    putCP437ch(uc);

    if (console_current == console_displayed)
        vga_updateCursor();
    mutex_unlock(&console_current->mutex);
}

static size_t atomic_puts(const char* text)
{
    size_t count = 0;
    for (; *text; ++text, ++count)
        atomic_putch(*text);

    if (console_current == console_displayed)
        vga_updateCursor();

    return count;
}

void puts(const char* text)
{
    mutex_lock(&console_current->mutex);
    atomic_puts(text);
    mutex_unlock(&console_current->mutex);
}

static void scroll(void)
{
    mutex_lock(&console_current->mutex);

    if ((console_current->properties & CONSOLE_AUTOSCROLL) && scroll_flag)
    {
        uint8_t scroll_begin = console_current->scrollBegin;
        uint8_t scroll_end = min(console_current->scrollEnd, ((console_current->properties & CONSOLE_FULLSCREEN)?LINES:(USER_END-USER_BEGIN)));

        if (console_current->cursor.y >= scroll_end)
        {
            uint8_t lines = console_current->cursor.y - scroll_end + 1U;
            memcpy((uint16_t*)console_current->vidmem + scroll_begin*COLUMNS, (uint16_t*)console_current->vidmem + scroll_begin*COLUMNS + lines * COLUMNS, (scroll_end - lines) * COLUMNS * sizeof(uint16_t));
            memsetw((uint16_t*)console_current->vidmem + (scroll_end - lines) * COLUMNS, getTextColor() << 8, COLUMNS);
            console_current->cursor.y = scroll_end - 1;

            if (console_current == console_displayed && (console_current->properties & CONSOLE_AUTOREFRESH))
            {
                refreshUserScreen();
                vga_updateCursor();
            }
        }
    }

    mutex_unlock(&console_current->mutex);
}

/// TODO: make it standardized!
// vprintf(...): supports %u, %d/%i, %f, %y/%x/%X, %s, %c, %% and the PrettyOS-specific %v, %I4, %I6 and %M
size_t vprintf(const char* args, va_list ap)
{
    mutex_lock(&console_current->mutex);

    uint8_t attribute = getTextColor();
    char buffer[32]; // Larger is not needed at the moment

    size_t pos; // variable used for return value

    for (pos = 0; *args; ++args)
    {
        switch (*args)
        {
            case '%':
                switch (*(++args))
                {
                    case 'u':
                        utoa(va_arg(ap, uint32_t), buffer);
                        pos += atomic_puts(buffer);
                        break;
                    case 'f':
                        ftoa(va_arg(ap, double), buffer);
                        pos += atomic_puts(buffer);
                        break;
                    case 'i': case 'd':
                        itoa(va_arg(ap, int32_t), buffer);
                        pos += atomic_puts(buffer);
                        break;
                    case 'X': /// TODO: make it standardized
                        i2hex(va_arg(ap, uint32_t), buffer, 8);
                        pos += atomic_puts(buffer);
                        break;
                    case 'x':
                        i2hex(va_arg(ap, uint32_t), buffer, 4);
                        pos += atomic_puts(buffer);
                        break;
                    case 'y':
                        i2hex(va_arg(ap, uint32_t), buffer, 2);
                        pos += atomic_puts(buffer);
                        break;
                    case 's':
                    {
                        char* temp = va_arg(ap, char*);
                        pos += atomic_puts(temp);
                        break;
                    }
                    case 'c':
                        atomic_putch((int8_t)va_arg(ap, int32_t));
                        pos++;
                        break;
                    case 'v':
                        textColor((attribute >> 4) | (attribute << 4));
                        atomic_putch(*(++args));
                        textColor(attribute);
                        pos++;
                        break;
                    case 'S': // Size: prints a size in bytes. Can autoscale them to KiB, MiB and GiB
                    {
                        uint64_t size = va_arg(ap, uint64_t);
                        switch (toLower(*++args)) // Second letter determines several properties:
                        {
                            case 'g': // GiB at maximum
                            case 'a': // autodetect
                                if ((size+0x2000000)/1024/1024/1024 >= 10)
                                {
                                    pos+=printf("%u GiB", (size+0x2000000)/1024/1024/1024);
                                    break;
                                }
                            case 'm': // MiB at maximum
                                if ((size+0x80000)/1024/1024 >= 10)
                                {
                                    pos+=printf("%u MiB", (size+0x80000)/1024/1024);
                                    break;
                                }
                            case 'k': // KiB at maximum
                                if ((size+0x200)/1024 >= 10)
                                    pos+=printf("%u KiB", (size+0x200)/1024);
                                else
                                    pos+=printf("%u B", size);
                                break;
                        }
                        break;
                    }
                    case 'I': // IP address
                    {
                        char type = *(++args);
                        if (type == '4')
                        {
                            IP4_t IP = va_arg(ap, IP4_t);
                            pos += printf("%u.%u.%u.%u", IP.IP4[0], IP.IP4[1], IP.IP4[2], IP.IP4[3]);
                        }
                        else if (type == '6')
                        {
                            IP6_t IP = va_arg(ap, IP6_t);
                            pos += printf("%x:%x:%x:%x:%x:%x:%x:%x", IP.IP6[0], IP.IP6[1], IP.IP6[2], IP.IP6[3], IP.IP6[4], IP.IP6[5], IP.IP6[6], IP.IP6[7]);
                        }
                        break;
                    }
                    case 'M': // MAC address
                    {
                        uint8_t* MAC = va_arg(ap, uint8_t*);
                        pos += printf("%y-%y-%y-%y-%y-%y", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
                        break;
                    }
                    case '%':
                        atomic_putch('%');
                        pos++;
                        break;
                    default:
                        --args;
                        --pos;
                        break;
                }
                break;
            default:
                atomic_putch(*args);
                pos++;
                break;
        }
    }

    if (console_current == console_displayed)
        vga_updateCursor();
    mutex_unlock(&console_current->mutex);
    return (pos);
}

size_t printf(const char* args, ...)
{
    va_list ap;
    va_start(ap, args);
    size_t retval = vprintf(args, ap);
    va_end(ap);
    return (retval);
}

size_t printfe(const char* args, ...)
{
    uint8_t col_old = currentTask->attrib;
    currentTask->attrib = ERROR;
    va_list ap;
    va_start(ap, args);
    size_t retval = vprintf(args, ap);
    va_end(ap);
    currentTask->attrib = col_old;
    return (retval);
}

size_t cprintf(const char* message, uint16_t line, int attribute, ...)
{
    mutex_lock(&console_current->mutex);
    uint8_t old_attrib = getTextColor();
    position_t cOld = console_current->cursor;
    scroll_flag = false;

    textColor(attribute);
    console_current->cursor.x = 0;
    console_current->cursor.y = line;

    // Call usual printf routines
    va_list ap;
    va_start(ap, attribute);
    size_t retval = vprintf(message, ap);
    va_end(ap);

    scroll_flag = true;
    textColor(old_attrib);
    console_current->cursor = cOld;
    mutex_unlock(&console_current->mutex);

    return (retval);
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
