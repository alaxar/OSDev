/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "power_management.h"
#include "util/util.h"
#include "video/video.h"
#include "video/console.h"
#include "tasking/vm86.h"


// No power management

static bool nopm_action(PM_STATES state)
{
    switch (state)
    {
        case PM_SOFTOFF: // Implemented by "hack", just as a fallback.
            cli();
            vga_clearScreen();
            vga_fillLine(' ', 0x20, 24);
            kprintf("                     Your computer can now be switched off.                     ", 25, 0x20);
            vga_fillLine(' ', 0x20, 26);
            hlt();
            return (false); // Hopefully not reached
        case PM_REBOOT: // We do not use the powermanagement here because its not necessary
        {
            int32_t temp; // A temporary int for storing keyboard info. The keyboard is used to reboot
            do // Flush the keyboard controller
            {
                temp = inportb(0x64);
                if (temp & 1)
                    inportb(0x60);
            }
            while (temp & 2);

            // Reboot
            outportb(0x64, 0xFE);

            return (false); // Hopefully not reached
        }
        default: // Every other state is unreachable without PM
            return (false);
    }
}



// APM (http://www.lowlevel.eu/wiki/APM)

extern uintptr_t apm_com_start;
extern uintptr_t apm_com_end;
// This values are hardcoded adresses from documentation/apm.map
#define APM_CHECK    ((void(*)(void))0x4000)
#define APM_INSTALL  ((void(*)(void))0x4027)
#define APM_SETSTATE ((void(*)(void))0x409D)

static pageDirectory_t* apm_pd;

static bool apm_install(void)
{
    apm_pd = paging_createPageDirectory();
    vm86_initPageDirectory(apm_pd, (void*)0x4000, &apm_com_start, (uintptr_t)&apm_com_end - (uintptr_t)&apm_com_start);

  #ifdef _DIAGNOSIS_
    textColor(HEADLINE);
    printf("\nAPM: ");
    textColor(TEXT);
  #endif
    // Check for APM
    *(uint8_t*)0x3400 = 0xFF;
    vm86_executeSync(apm_pd, APM_CHECK);
    if (*((uint8_t*)0x3400) != 0) // Error
    {
      #ifdef _DIAGNOSIS_
        printf("\nNot available.");
      #endif
        return (false);
    }
  #ifdef _DIAGNOSIS_
    printf("\nVersion: %u.%u, Control string: %c%c, Flags: %x.", *((uint8_t*)0x3402), *((uint8_t*)0x3401), *((uint8_t*)0x3404), *((uint8_t*)0x3403), *((uint16_t*)0x3405));
  #endif

    // Activate APM
    *(uint8_t*)0x3400 = 1;
    vm86_executeSync(apm_pd, APM_INSTALL);
    switch (*((uint8_t*)0x3400))
    {
        case 0:
          #ifdef _DIAGNOSIS_
            printf("\nSuccessfully activated.");
          #endif
            return (true);
        case 1:
            printfe("\nAPM: Error while disconnecting: %yh.", *((uint8_t*)0x3401));
            return (false);
        case 2:
            printfe("\nAPM: Error while connecting: %yh.", *((uint8_t*)0x3401));
            return (false);
        case 3:
            printfe("\nAPM: Error while handling out APM version: %yh.", *((uint8_t*)0x3401));
            return (false);
        case 4:
            printfe("\nAPM: Error while activating: %yh.", *((uint8_t*)0x3401));
            return (false);
    }
    return (false);
}

static bool apm_action(PM_STATES state)
{
    switch (state)
    {
        case PM_STANDBY:
            *((uint16_t*)0x3400) = 2; // Suspend-Mode (turns more hardware off than standby)
            vm86_executeSync(apm_pd, APM_SETSTATE);
            return (*((uint16_t*)0x3400) != 0);
        case PM_SOFTOFF:
            *((uint16_t*)0x3400) = 3;
            vm86_executeSync(apm_pd, APM_SETSTATE);
            return (*((uint16_t*)0x3400) != 0);
        default: // Every other state is unreachable with APM
            return (false);
    }
}


// Interface

static PM_SYSTEM_t powmgmt_systems[_PM_SYSTEMS_END] =
{
    {.action = &nopm_action, .supported = true}, // No PM, always available
    {.action = &apm_action},                     // APM
    {.action = 0}                                // ACPI
};

void powmgmt_install(void)
{
    powmgmt_systems[PM_APM].supported = apm_install();
    powmgmt_systems[PM_ACPI].supported = false; // Unsupported by PrettyOS
}

void powmgmt_log(void)
{
    textColor(LIGHT_GRAY);
    printf("   => APM: ");
    textColor(TEXT);
    puts(powmgmt_systems[PM_APM].supported?"Available":"Not supported");
    textColor(LIGHT_GRAY);
    /*printf("\n   => ACPI: ");
    textColor(TEXT);
    puts("Not supported by PrettyOS");*/
}

bool powmgmt_action(PM_STATES state)
{
    if (state < _PM_STATES_END)
    {
        bool success = false;
        for (int32_t i = _PM_SYSTEMS_END-1; i >= 0 && !success; i--) // Trying out all supported power management systems.
        {
            if (powmgmt_systems[i].supported && powmgmt_systems[i].action != 0)
                success = powmgmt_systems[i].action(state);
        }
        return (success);
    }
    return (false); // Invalid state
}

/*
* Copyright (c) 2010-2017 The PrettyOS Project. All rights reserved.
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
