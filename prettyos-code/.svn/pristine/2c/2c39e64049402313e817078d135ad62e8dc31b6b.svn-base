/*
 *  license and disclaimer for the use of this source code as per statement below
 *  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
 */

// Utilities
#include "util/util.h"          // sti, memset, strcmp, strlen, rdtsc, ...
#include "util/todo_list.h"     // todoList_execute
#include "log.h"                // log_printf, log_incrementScreenLevel, ...

// Internal devices
#include "cpu.h"                // cpu_analyze
#include "cmos.h"               // cmos_read
#include "timer.h"              // timer_install, timer_getSeconds, sleepMilliSeconds
#include "time.h"               // getCurrentDateAndTime
#include "descriptor_tables.h"  // idt_install, gdt_install
#include "irq.h"                // isr_install
#include "power_management.h"   // powmgmt_install, powmgmt_log
#include "apic.h"               // lapic_install,ioapic_install, ...
#include "acpi/acpi.h"          // acpi_install

// Base system
#include "kheap.h"              // heap_install, malloc, free, logHeapRegions
#include "tasking/task.h"       // tasking_install & others
#include "syscall.h"            // syscall_install
#include "ipc.h"                // ipc_print

// External devices
#include "cdi.h"                // cdi_init
#include "hid/keyboard.h"       // keyboard_install, KEY_...
#include "hid/mouse.h"          // mouse_install
#include "audio/audio.h"        // audio_test
#include "serial.h"             // serial_init
#include "video/videomanager.h" // video_install, video_test
#include "filesystem/ramdisk.h" // ramdisk_install, ramdisk_loadShell
#include "storage/ata.h"        // ata_install
#include "storage/flpydsk.h"    // flpydsk_install
#include "usb/usb_hub.h"        // usb_hubWatchdog
#ifdef _ENABLE_HDD_
#include "storage/hdd.h"        // hdd_install
#endif

// Network
#include "netprotocol/tcp.h"    // tcp_showConnections, network_displayArpTables


const char* const version = "0.0.5.129 - Rev: 1795";

// .bss
extern char _bss_start; // Linker script
extern char _bss_end;   // Linker script

todoList_t kernel_idleTasks = {list_init(), mutex_init()}; // List of functions that are executed in kernel idle loop

static void logText(const char* str)
{
    textColor(LIGHT_GRAY);
    size_t len = printf("   => %s: ", str);

    for (size_t i = len; i < 25; i++)
        putch(' ');
}

static void logExec(bool b)
{
    putch('[');
    if (b)
    {
        textColor(SUCCESS);
        printf("OK");
    }
    else
    {
        printfe("ERROR");
    }
    textColor(LIGHT_GRAY);
    printf("]\n");
    textColor(TEXT);
}

#define log(Text, Func) {logText(Text); logExec(Func);} // For functions returning bool. Writes [ERROR] if false is returned. [OK] otherwise.
#define simpleLog(Text, Func) {logText(Text); Func; logExec(true);} // For functions returning void. Writes [OK]


typedef struct // http://www.lowlevel.eu/wiki/Multiboot
{
    uint32_t flags;
    uint32_t memLower;
    uint32_t memUpper;
    uint32_t bootdevice;
    uint32_t cmdline;
    uint32_t modsCount;
    void*    mods;
    uint32_t syms[4];
    uint32_t mmapLength;
    void*    mmap;
    uint32_t drivesLength;
    void*    drives;
    uint32_t configTable;
    char*    bootloaderName;
    uint32_t apmTable;
    uint32_t vbe_controlInfo;
    uint32_t vbe_modeInfo;
    uint16_t vbe_mode;
    uint16_t vbe_interfaceSeg;
    uint16_t vbe_interfaceOff;
    uint16_t vbe_interfaceLen;
} __attribute__((packed)) multiboot_t;

static void init(multiboot_t* mb_struct)
{
    // Set .bss to zero
    memset(&_bss_start, 0, (uintptr_t)&_bss_end - (uintptr_t)&_bss_start);

    // Prepare video
    kernel_console_init();
    vga_clearScreen();

    textColor(HEADLINE);
    printf(" => Initializing PrettyOS:\n\n");

    gdt_install();
    isr_install();
    cpu_install();

    // Memory
    int64_t memsize = paging_install(mb_struct->mmap, mb_struct->mmapLength);
    ipc_setInt("PrettyOS/RAM", &memsize, IPC_SIZE);
    log("Paging", memsize != 0);
    simpleLog("Heap", heap_install());

    if (acpi_available())
        simpleLog("ACPI", acpi_install()); // Set up APIC driver and execute base parsers

    simpleLog("PIC", pic_remap()); // Cf. interrupts.asm

    simpleLog("PCI", pci_scan()); // Early scan of PCI bus to detect PCI-to-ISA bridge!

    bool lapic = lapic_available();
    if (lapic)
        log("Local APIC", lapic_install());
    if (ioapic_available())
        log("I/O APIC", ioapic_install());
    sti();

    log("FPU", fpu_install());

    simpleLog("Timer", timer_install(SYSTEMFREQUENCY, !lapic)); // Sets system frequency to ... Hz

    simpleLog("Multitasking", tasking_install());

    log("Video", vga_install(strcmp(mb_struct->bootloaderName, "PrettyBL") == 0));

  #ifdef _BOOTSCREEN_
    scheduler_insertTask(create_cthread(&bootscreen, "Booting ..."));
  #endif

    int64_t bootArchitecture = BIT(1);
    ipc_getInt("PrettyOS/ACPI/fadt/boot_architecture", &bootArchitecture);
    if (bootArchitecture & BIT(1))
    {
        simpleLog("Mouse", mouse_install());
        simpleLog("Keyboard", keyboard_install());
    }

    simpleLog("Power Management", powmgmt_install());

    simpleLog("Syscalls", syscall_install());

    simpleLog("Devicemanager", deviceManager_install());

    puts("\n\n");
}

static void showMemorySize(void)
{
    textColor(LIGHT_GRAY);
    printf("   => Memory: ");
    textColor(TEXT);
    int64_t ramsize;
    ipc_getInt("PrettyOS/RAM", &ramsize);

    printf("%Sa (%u Bytes)\n", ramsize, (uint32_t)ramsize);
    textColor(LIGHT_GRAY);
}

void main(multiboot_t* mb_struct)
{
    init(mb_struct);
    srand(cmos_read(CMOS_SECOND));

    textColor(HEADLINE);
    printf(" => System analysis:\n");

    showMemorySize();
    cpu_analyze();
    fpu_test();
    powmgmt_log();

    // Initialize advanced devices
    serial_init();
    pci_installDevices(); // Install (already detected) PCI devices
    cdi_init(); // http://www.lowlevel.eu/wiki/CDI
    flpydsk_install(); // Detect FDDs

  #ifdef _ENABLE_HDD_
    ata_install();
    hdd_install();
  #endif

    ramdisk_install();

  #ifdef _DEVMGR_DIAGNOSIS_
    deviceManager_showPortList();
    deviceManager_showDiskList();
  #endif

    ramdisk_loadShell();

    textColor(SUCCESS);
    printf("\n\n--------------------------------------------------------------------------------");
    printf("                                PrettyOS Booted\n");
    printf("--------------------------------------------------------------------------------");
    textColor(TEXT);

    const char* progress    = "|/-\\";    // Rotating asterisk
    uint32_t CurrentSeconds = 0xFFFFFFFF; // Set on a high value to force a refresh of the statusbar at the beginning.

    bool ESC   = false; // key Esc
    bool CTRL  = false; // key Ctrl
    bool FOCUS_SCREEN = false; // screen log focus key code
    bool FOCUS_SECONDARY = false; // secondary log focus key code
    ////////////////////////////////////////////////////////
    // Start of kernel idle loop

    while (true)
    {
        // Show rotating asterisk. Write the character on the screen. (color|character)
        if (!(console_displayed->properties & CONSOLE_FULLSCREEN))
            vga_setPixel(79, 49, (FOOTNOTE<<8) | *(const uint8_t*)progress);

        if (*++progress == 0)
            progress-=4; // Reset asterisk

        // Handle events. TODO: Some of the shortcuts can be moved to the shell later.
        union {
            char c;
            KEY_t key;
        } buffer;
        EVENT_t ev = event_poll(&buffer, sizeof(buffer), EVENT_NONE); // Take one event from the event queue

        while (ev != EVENT_NONE)
        {
            switch (ev)
            {
                case EVENT_KEY_DOWN:
                    // Detect CTRL and ESC
                    switch (buffer.key)
                    {
                        case KEY_ESC:
                            ESC = true;
                            break;
                        case KEY_LCTRL:
                            CTRL  = true;
                            break;
                        case KEY_RCTRL:
                            CTRL  = true;
                            break;
                        default:
                            break;
                    }
                    if (ESC)
                    {
                        switch (buffer.key)
                        {
                            case KEY_F1:
                                log_screen = !log_screen;
                                log_showScreenStatus();
                                break;
                            case KEY_F2:
                                log_incrementScreenLevel();
                                break;
                            case KEY_F3:
                                log_decrementScreenLevel();
                                break;
                            case KEY_F4:
                                FOCUS_SCREEN = true;
                                writeInfo(2, "Enter character to select focus!");
                                break;
                            case KEY_F5:
                                log_secondary = !log_secondary;
                                log_showSecondaryStatus();
                                break;
                            case KEY_F6:
                                log_incrementSecondaryLevel();
                                break;
                            case KEY_F7:
                                log_decrementSecondaryLevel();
                                break;
                            case KEY_F8:
                                FOCUS_SECONDARY = true;
                                writeInfo(2, "Enter character to select focus!");
                                break;
                            case KEY_F9:
                                printf("\nTODO: select output for secondary log");
                                break;
                            case KEY_F10:
                                log_toggleSecondaryMode();
                                break;
                            case KEY_F11:
                                log_flushSecondary();
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case EVENT_KEY_UP:
                    // Detect CTRL and ESC
                    switch (buffer.key)
                    {
                        case KEY_ESC:
                            ESC = false;
                            break;
                        case KEY_LCTRL:
                            CTRL  = false;
                            break;
                        case KEY_RCTRL:
                            CTRL  = false;
                            break;
                        default:
                            break;
                    }
                    break;
                case EVENT_TEXT_ENTERED:
                    if (FOCUS_SCREEN)
                    {
                        log_setScreenFocus(buffer.c);
                        FOCUS_SCREEN = false;
                    }
                    else if (FOCUS_SECONDARY)
                    {
                        log_setSecondaryFocus(buffer.c);
                        FOCUS_SECONDARY = false;
                    }
                    else if (ESC)
                    {
                        switch (buffer.c)
                        {
                            case 'h': heap_logRegions();              break;
                            case 'p': paging_analyzeBitTable();       break;
                        }
                    }
                    else if (CTRL)
                    {
                        switch (buffer.c)
                        {
                            case 'a': network_displayArpTables();     break;
                            case 'c': tcp_showConnections();          break;
                            case 'd': deviceManager_showPortList();
                                      deviceManager_showDiskList();   break;
                            case 'f': fsmanager_log();                break;
                            case 'i': ipc_print();                    break;
                            case 's': ramdisk_loadShell();            break;
                            case 't': scheduler_log();                break;
                            case 'v': scheduler_insertTask(create_cthread(&video_test, "VBE")); break;
                            case 'w': scheduler_insertTask(create_cthread(&audio_test, "Audio")); break;
                        }
                    }
                    // In case that waitForKeyStroke() was used inside of the functions above, we have to manually refresh following variables, since waitForKeyStroke() "eats" all events
                    ESC   = keyPressed(KEY_ESC);
                    CTRL = keyPressed(KEY_LCTRL) || keyPressed(KEY_RCTRL);
                    break;
                case EVENT_CONSOLE_LOST_FOCUS:
                    // Clear key status
                    ESC = false;
                    CTRL = false;
                    FOCUS_SCREEN = false;
                    FOCUS_SECONDARY = false;
                    break;
                default:
                    break;
            }//switch
            ev = event_poll(&buffer, sizeof(buffer), EVENT_NONE);
        }//while

        if (timer_getSeconds() != CurrentSeconds) // Execute one time per second
        {
            CurrentSeconds = timer_getSeconds();
            cpu_calculateFrequency();

            if (!(console_displayed->properties & CONSOLE_FULLSCREEN))
            {
                // Draw status bar with date, time and frequency
                char DateAndTime[50];
                getCurrentDateAndTime(DateAndTime, 50);
                kprintf("%s   %u s runtime. CPU: %u MHz    ", 49, FOOTNOTE, DateAndTime, CurrentSeconds, ((uint32_t)*cpu_frequency)/1000); // Output in status bar
            }

          #ifdef _IRQ_DIAGNOSIS_
            char str[80];
            uint32_t* nrInt = (uint32_t*)irq_getNumberOfInterrupts();
            snprintf(str, 80, "int/sec: all: %u sysc: %u timer: %u switch: %u", nrInt[0], nrInt[3], nrInt[1], nrInt[2]);
            writeInfo(2, str);
          #endif

            deviceManager_checkDrives(); // Switch off motors if they are not neccessary; check UHCI port changes, ...

            if ((CurrentSeconds > 5) && (CurrentSeconds%2)) // wait some time after boot-up and check usb-Hubs every 2 sec for port connected status changes
            {
                usb_hubWatchdog();
            }
        }

        usb_pollInterruptTransfers();

        if (serial_received(1) != 0)
        {
            textColor(HEADLINE);
            printf("\nSerial message: ");
            textColor(DATA);

            do
            {
                printf("%y ", serial_read(1));
                sleepMilliSeconds(8);
            } while (serial_received(1) != 0);
        }

        todoList_execute(&kernel_idleTasks);
        switch_context(false); // Kernel idle loop has finished so far. Provide time to next waiting task
    }

    // End of kernel idle loop
    ////////////////////////////////////////////////////////
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
