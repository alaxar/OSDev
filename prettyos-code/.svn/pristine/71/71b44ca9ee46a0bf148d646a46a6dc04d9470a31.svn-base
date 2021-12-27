/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cdi/storage.h"


void cdi_storage_driver_init(struct cdi_storage_driver* driver)
{
    driver->drv.type = CDI_STORAGE;
    cdi_driver_init((struct cdi_driver*) driver);
}

void cdi_storage_driver_destroy(struct cdi_storage_driver* driver)
{
    cdi_driver_destroy((struct cdi_driver*) driver);
}

void cdi_storage_device_init(struct cdi_storage_device* device)
{
    ///cdi_list_t partitions;
    ///struct partition* part;

    // Geraeteknoten fuer LostIO erstellen
    ///part = calloc(1, sizeof(*part));
    ///part->dev       = device;
    ///part->number    = (uint16_t) -1;
    ///part->start     = 0;
    ///part->size      = device->block_size * device->block_count;

    ///lostio_mst_if_newdev(part);

    // Partitionen suchen und Knoten erstellen
    ///partitions = cdi_list_create();
    ///cdi_tyndur_parse_partitions(device, partitions);
    ///while ((part = cdi_list_pop(partitions))) {
        ///lostio_mst_if_newdev(part);
    ///}
    ///cdi_list_destroy(partitions);
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
