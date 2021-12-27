/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "ramdisk.h"
#include "util/util.h"
#include "video/console.h"
#include "storage/devicemanager.h"
#include "tasking/task.h"
#include "kheap.h"
#include "ipc.h"
#include "executable.h"


// file and data going to RAM disk
extern char shell_start;
extern char shell_end;
static ramdisk_node_t shellNode =
{
    .node = {
        .attributes = NODE_SYSTEM | NODE_READONLY,
        .name = "SHELL.ELF"
    }
};


static void installDisk(disk_t* disk, size_t partitionID)
{
    disk->size = (size_t)(&shell_end - &shell_start)*sizeof(shell_start); // HACK

    disk->partition[partitionID]         = malloc(sizeof(partition_t), 0, "ramdisk-part");
    disk->partition[partitionID]->disk   = disk;
    disk->partition[partitionID]->data   = list_create(); // data pointer contains list of file_t
    disk->partition[partitionID]->mount  = true;
    disk->partition[partitionID]->type   = &RAMFS;
    disk->partition[partitionID]->serial = 0;
    disk->partition[partitionID]->name   = "RAMDISK";
    disk->partition[partitionID]->rootFolder = 0; // HACK

    // Create/initialize initial data
    shellNode.data = &shell_start;
    shellNode.node.size = (size_t)(&shell_end - &shell_start)*sizeof(shell_start);
    shellNode.allocated_size = shellNode.node.size;

    // "Load" initial data (shell) into ramdisk/IPC
    list_t* nodelist = disk->partition[partitionID]->data;
    list_append(nodelist, &shellNode);
    ipc_node_t* node;
    ipc_createNode("PrettyOS/Shell.ELF", &node, IPC_FILE);
    node->data.file = &shellNode;

    systemPartition = disk->partition[partitionID]; // Use ramdisk as system partition
}

void ramdisk_install(void)
{
    static port_t RAMport =
    {
        .type = &RAM,
        .insertedDisk = 0, // set later
        .data = 0,
        .name = "RAM"
    };
    static disk_t RAMdisk =
    {
        .type = &RAMDISK,
        .partition = { 0, 0, 0, 0 },
        .size = 0,
        .data = 0,
        .accessRemaining = 0,
        .port = &RAMport,
        .sectorSize = 0,
        .secPerTrack = 0,
        .headCount = 0,
        .optAccSecCount = 0,
        .alignedAccess = false,
        .BIOS_driveNum = 0
    };
    RAMport.insertedDisk = &RAMdisk;
    deviceManager_attachDisk(&RAMdisk);
    deviceManager_attachPort(&RAMport);

    installDisk(&RAMdisk, 0);
}

file_t* ramdisk_openFile(ramdisk_node_t* node, const char* path, const char* mode)
{
    file_t* file = malloc(sizeof(file_t), 0, "ramdisk-file");
    file->volume = systemPartition;
    file->seek = 0;
    file->folder = file->volume->rootFolder; // HACK. Not all files are in the root folder
    file->size = node->node.size;
    file->EOF = false;
    file->error = CE_GOOD;
    file->name = strdup(getFilename(path), "ramdisk-filename");
    file->data = node;

    //bool create = true;
    switch (mode[0])
    {
    case 'W': case 'w':
        file->write = true;
        file->read = mode[1] == '+';
        break;
    case 'A': case 'a':
        file->write = true;
        file->read = mode[1] == '+';
        ramdisk_fseek(file, 0, SEEK_END);
        break;
    case 'R': case 'r':
        file->read = true;
        //create = false;
        file->write = mode[1] == '+';
        break;
    default: // error
        file->read = false;
        file->write = false;
        break;
    }

    if (currentTask->files == 0)
        currentTask->files = list_create();
    list_append(currentTask->files, file);

    return file;
}

FS_ERROR ramdisk_fclose(file_t* file)
{
    return CE_GOOD;
}

FS_ERROR ramdisk_fseek(file_t* file, int32_t offset, SEEK_ORIGIN origin)
{
    ramdisk_node_t* ramfile = file->data;
    int64_t newPos = file->seek;
    switch (origin)
    {
        case SEEK_SET:
            newPos = offset;
            break;
        case SEEK_CUR:
            newPos += offset;
            break;
        case SEEK_END:
            newPos = (int64_t)ramfile->node.size - offset;
            break;
        default:
            return CE_INVALID_ARGUMENT;
    }
    if (newPos < 0 || newPos >= (int64_t)ramfile->node.size)
        return CE_INVALID_ARGUMENT;
    file->seek = newPos;
    return CE_GOOD;
}

FS_ERROR ramdisk_fread(file_t* file, void* destination, size_t size)
{
    ramdisk_node_t* ramfile = file->data;
    bool eof = file->seek + size >= ramfile->node.size;
    size_t toBeRead = size;
    if (eof)
        toBeRead = ramfile->node.size - file->seek;

    memcpy(destination, ramfile->data + file->seek, toBeRead);
    file->seek += toBeRead;

    if (eof)
        return CE_EOF;
    return CE_GOOD;
}

FS_ERROR ramdisk_fwrite(file_t* file, const void* source, size_t size)
{
    ramdisk_node_t* ramfile = file->data;
    if (file->seek < ramfile->allocated_size && size > 0) // Overwrite
    {
        size_t bytes = min(file->size - file->seek, size);
        memcpy(ramfile->data, source, bytes);
        size -= bytes;
        file->seek += bytes;
        file->size = max(file->size, file->seek);
    }
    if (size > 0) // Append what is left
    {
        size_t newsize = file->size + size;
        size_t allocate = alignUp(newsize, PAGESIZE);
        void* new = malloc(allocate, 0, "ramfile data"); // TODO: realloc
        if (!new)
            return CE_DISK_FULL;
        ramfile->allocated_size = allocate;
        memcpy(new, ramfile->data, file->size);
        free(ramfile->data);
        ramfile->data = new;
        memcpy(new + file->size, source, size);
        file->size = file->seek = newsize;
    }
    return CE_GOOD;
}

FS_ERROR ramdisk_fflush(file_t* file)
{
    return CE_GOOD; // Nothing to do - no caching in ramdisk
}

bool ramdisk_loadShell(void)
{
    if (executeFile("0:0:|PrettyOS|Shell.ELF", 0, 0, false) != CE_GOOD)
    {
        printfe("\nERROR: shell cannot be started.");
        return false;
    }
    return true;
}

FS_ERROR ramdisk_folderAccess(folder_t* folder, folderAccess_t mode)
{
    printfe("\nNot yet supported! Try crtl+I");
    return CE_UNSUPPORTED_FUNCTION;
}


/*
* Copyright (c) 2013-2015 The PrettyOS Project. All rights reserved.
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
