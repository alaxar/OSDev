/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cdi.h"
#include "util/list.h"
#include "cdi/pci.h"
#include "cdi/storage.h"
#include "cdi/fs.h"
#include "video/console.h"

static cdi_list_t drivers = 0;

extern struct cdi_driver* _cdi_start; // Declared in kernel.ld
extern struct cdi_driver* _cdi_end;   // Declared in kernel.ld

// Initialisiert alle PCI-Geraete
static void init_pci_devices(void)
{
    // Liste der PCI-Geraete holen
    cdi_list_t pciDevices = cdi_list_create();
    cdi_pci_get_all_devices(pciDevices);

    struct cdi_pci_device* pci;
    // Fuer jedes Geraet einen Treiber suchen
    for (int i = 0; (pci = cdi_list_get(pciDevices, i)); i++)
    {
        struct cdi_device* device = 0;
        struct cdi_driver* driver;
        for (size_t j = 0; (driver = cdi_list_get(drivers, j)); j++)
        {
            if (driver->bus == CDI_PCI && driver->init_device)
            {
                pci->meta.driver = driver;
                device = driver->init_device(&pci->bus_data);
                pci->meta.cdiDev = device;
                break;
            }
        }

        if (device != 0)
        {
            cdi_list_push(driver->devices, device);
        }
        else
        {
            cdi_pci_device_destroy(pci);
        }
    }

    cdi_list_destroy(pciDevices);
}

/* Diese Funktion wird von Treibern aufgerufen, nachdem ein neuer Treiber
   hinzugefuegt worden ist.

   Sie registriert typischerweise die neu hinzugefuegten Treiber und/oder
   Geraete beim Betriebssystem und startet damit ihre Ausfuehrung.

   Nach dem Aufruf dieser Funktion duerfen vom Treiber keine weiteren Befehle
   ausgefuehrt werden, da nicht definiert ist, ob und wann die Funktion
   zurueckkehrt. */
static void run_drivers(void)
{
    // PCI-Geraete suchen
    init_pci_devices();

    // Geraete initialisieren
    struct cdi_driver* driver;
    for (size_t i = 0; (driver = cdi_list_get(drivers, i)); i++)
    {
        struct cdi_device* device;
        for (size_t j = 0; (device = cdi_list_get(driver->devices, j)); j++)
        {
            device->driver = driver;
        }
    }
}

void cdi_init()
{
    // Interne Strukturen initialisieren
    drivers = cdi_list_create();

    // Alle in dieser Binary verfuegbaren Treiber aufsammeln
    struct cdi_driver** pdrv = &_cdi_start;
    while (pdrv < &_cdi_end)
    {
        struct cdi_driver* drv = *pdrv;
        if (drv->init != 0)
        {
            drv->init();
            cdi_driver_register(drv);
        }
        pdrv++;
    }

    // Treiber starten
    run_drivers();
}

void cdi_driver_init(struct cdi_driver* driver)
{
    driver->devices = cdi_list_create();
}

void cdi_driver_destroy(struct cdi_driver* driver)
{
    cdi_list_destroy(driver->devices);
}

void cdi_driver_register(struct cdi_driver* driver)
{
    cdi_list_push(drivers, driver);

    switch (driver->type)
    {
        case CDI_STORAGE:
            //cdi_storage_driver_register((struct cdi_storage_driver*) driver);
            break;
        case CDI_FILESYSTEM:
            cdi_fs_driver_register((struct cdi_fs_driver*) driver);
            break;
        default:
            break;
    }
}

int cdi_provide_device(struct cdi_bus_data* device);

int cdi_provide_device_internal_drv(struct cdi_bus_data* device, struct cdi_driver* driver);

void cdi_handle_bus_device(struct cdi_driver* drv, struct cdi_bus_device_pattern* pattern);

/* Wenn main nicht von einem Treiber ueberschrieben wird, ist hier der
   Einsprungspunkt. Die Standardfunktion ruft nur cdi_init() auf. Treiber, die
   die Funktion ueberschreiben, koennen argc und argv auswerten und muessen als
   letztes ebenfalls cdi_init aufrufen. */
int __attribute__((weak)) main(void)
{
    cdi_init();
    return (0);
}

/*
* Copyright (c) 2009-2016 The PrettyOS Project. All rights reserved.
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
