/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "fat12.h"
#include "video/console.h"
#include "filesystem/fsmanager.h"


int32_t flpydsk_read_directory(const char* path)
{
    textColor(HEADLINE);
    if (path == 0)
        puts("\n<Floppy Disk - Root Directory>");
    else
        printf("\n<%s - Directory>", path);
    textColor(TABLE_HEADING);
    puts("\nFile\t\tSize (Bytes)\tAttributes\n");
    textColor(TEXT);

    folder_t* folder = folderAccess(path?path:"1:0:", FOLDER_OPEN);
    if (!folder)
        return -1;

    for(dlelement_t* e = folder->nodes.head; e; e = e->next) {
        fsnode_t* node = e->data;

        size_t letters = printf("%s", node->name);
        if (letters < 8) putch('\t');

        // Filesize
        if (!(node->attributes & NODE_VOLUME) && !(node->attributes & NODE_DIRECTORY))
            printf("\t%u\t\t", node->size);
        else
            puts("\t\t\t");

        // Attributes
        if (node->attributes & NODE_VOLUME)    puts("(vol) ");
        if (node->attributes & NODE_DIRECTORY) puts("(dir) ");
        if (node->attributes & NODE_READONLY)  puts("(r/o) ");
        if (node->attributes & NODE_HIDDEN)    puts("(hid) ");
        if (node->attributes & NODE_SYSTEM)    puts("(sys) ");
        if (node->attributes & NODE_ARCHIVE)   puts("(arc)");

        putch('\n');
    }
    folderClose(folder);
    putch('\n');
    return (0);
}


/*
* Copyright (c) 2009-2014 The PrettyOS Project. All rights reserved.
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
