/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cdi/fs.h"


struct cdi_fs_driver* the_one_and_only_driver = 0;

void cdi_fs_driver_init(struct cdi_fs_driver* driver)
{
    static int initialized = 0;

    if (!initialized) {

        ///lostio_init();
        ///lostio_type_directory_use();
        ///lostio_register_typehandle(&cdi_typehandle);
        ///lostio_register_typehandle(&cdi_symlink_typehandle);
        ///lostio_register_typehandle(&cdi_null_typehandle);


        // Preopen-Handler fuer Verzeichnisse setzen
        ///typehandle_t* typehandle = get_typehandle(LOSTIO_TYPES_DIRECTORY);
        ///typehandle->pre_open = &lostio_pre_open_handler;
        ///typehandle->not_found = &lostio_not_found_handler;
        ///typehandle->post_open = 0;
        ///typehandle->unlink = &lostio_unlink_handler;

        initialized = 1;
    }
}

void cdi_fs_driver_destroy(struct cdi_fs_driver* driver)
{
}

void cdi_fs_driver_register(struct cdi_fs_driver* driver)
{
    if (the_one_and_only_driver) {
        return;
    }

    the_one_and_only_driver = driver;
}

size_t cdi_fs_data_read(struct cdi_fs_filesystem* fs, uint64_t start, size_t size, void* buffer);

size_t cdi_fs_data_write(struct cdi_fs_filesystem* fs, uint64_t start, size_t size, const void* buffer);

/*
* Copyright (c) 2009 The PrettyOS Project. All rights reserved.
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
