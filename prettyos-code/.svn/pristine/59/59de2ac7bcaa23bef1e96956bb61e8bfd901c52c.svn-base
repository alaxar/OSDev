/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "fsmanager.h"
#include "storage/devicemanager.h"
#include "fat.h"
#include "ramdisk.h"
#include "kheap.h"
#include "util/util.h"
#include "util/array.h"
#include "tasking/task.h"
#include "ipc.h"


const fileSystem_t FAT = {.fopen        = &FAT_fopen,
                          .fclose       = &FAT_fclose,
                          .fread        = &FAT_fread,
                          .fwrite       = &FAT_fwrite,
                          .fseek        = &FAT_fseek,
                          .fflush       = 0,
                          .remove       = &FAT_remove,
                          .rename       = &FAT_rename,
                          .pformat      = &FAT_format,
                          .pfree        = &FAT_pfree,
                          .pinstall     = &FAT_pinstall,
                          .folderAccess = &FAT_folderAccess,
                          .folderClose  = &FAT_folderClose};

const fileSystem_t RAMFS = {.fopen        = 0, // Special: Not necessary for RAMFS
                            .fclose       = &ramdisk_fclose,
                            .fread        = &ramdisk_fread,
                            .fwrite       = &ramdisk_fwrite,
                            .fseek        = &ramdisk_fseek,
                            .fflush       = 0,
                            .remove       = 0,
                            .rename       = 0,
                            .pformat      = 0,
                            .pinstall     = 0,
                            .folderAccess = ramdisk_folderAccess,
                            .folderClose  = 0};


static uint64_t getFSType(FS_t type); // BIT 0-31: subtype, BIT 32-63: fileSystem_t
static folder_t* getFolder(const char* path, partition_t** part, bool isAbsolute, bool folderPath);
static folder_t* folderAccess_internal(char* name, folder_t* parent, partition_t* volume, folderAccess_t mode);


////////////////////
// File functions //
////////////////////

const char* getFilename(const char* path)
{
    const char* temp = strpbrk(path, "/|\\");
    while (temp != 0 && temp[1] != 0)
    {
        path = temp + 1;
        temp = strpbrk(path, "/|\\");
    }
    return (path);
}

file_t* fopen(const char* path, const char* mode)
{
    partition_t* part = deviceManager_getPartition(path);

    if (part && part->type == &RAMFS)
    {
        file_t* file;
        path = strpbrk(path, "/|\\");
        if (!path)
            return 0;
        ipc_fopen(path+1, &file, mode);
        file->volume = part;
        return (file);
    }

    folder_t* parent = getFolder(path, &part, false, false);
    if (parent == 0 || part == 0)
        return (0);

    if (part->type->fopen == 0)
    {
        printfe("\nERROR: function fopen not defined");
        return (0);
    }

    file_t* file = malloc(sizeof(file_t), 0, "fsmgr-file");
    file->volume = part;
    file->seek   = 0;
    file->folder = parent;
    file->size   = 0; // Init with 0 but set in FS-specific fopen
    file->EOF    = false;
    file->error  = CE_GOOD;
    file->name   = strdup(getFilename(path), "fsmgr-filename");

    bool appendMode = false; // Used to seek to end
    bool create     = true;
    file->binary    = false;
    file->write     = false;
    file->read      = false;
    for (int i = 0; mode[i] != 0; i++)
    {
        char c = toLower(mode[i]);

        if(c == 'w')
            file->write = true;
        else if (c == 'a')
        {
            appendMode = true;
            file->write = true;
        }
        else if (c == 'r')
        {
            file->read = true;
            create = false;
        }
        else if (c == '+')
        {
            appendMode = true;
            file->write = true;
            file->read = true;
        }
        else if (c == 'b')
            file->binary = true;
    }

    FS_ERROR err = file->volume->type->fopen(file, create, !appendMode&&create);
    if (err != CE_GOOD)
    {
      #ifdef _DEVMGR_DIAGNOSIS_
        printfe("\nERROR: Opening file failed.");
      #endif
        free(file->name);
        free(file);
        return (0);
    }

    if (appendMode)
    {
        fseek(file, 0, SEEK_END);
    }

    if (currentTask->files == 0)
        currentTask->files = list_create();
    list_append(currentTask->files, file);

    return (file);
}

void fclose_task(file_t* file, task_t* task)
{
    file->volume->type->fclose(file);
    deviceManager_flushCaches(file->volume->disk);
    free(file->name);
    list_delete(task->files, list_find(task->files, file));
    free(file);

    if (list_isEmpty(task->files))
    {
        list_free(task->files);
        task->files = 0;
    }
}

void fclose(file_t* file)
{
    fclose_task(file, currentTask);
}

FS_ERROR fmove(const char* oldpath, const char* newpath, bool copy)
{
    if (newpath == 0)
    {
        file_t file = { 0 };
        file.volume = deviceManager_getPartition(oldpath);
        file.folder = getFolder(oldpath, &file.volume, false, false);
        file.name = (char*)getFilename(oldpath);
        FS_ERROR error = file.volume->type->remove(&file);
        deviceManager_flushCaches(file.volume->disk);
        return (error);
    }

    partition_t* spart = deviceManager_getPartition(oldpath);
    partition_t* dpart = deviceManager_getPartition(newpath);

    if (!copy && (spart == dpart || dpart == 0)) // same partition
    {
        folder_t* newFolder = getFolder(newpath, &spart, false, false);
        folder_t* oldFolder = getFolder(newpath, &spart, false, false);

        if (newFolder == 0 || oldFolder == 0)
            return CE_FILE_NOT_FOUND;

        file_t file = {0};
        file.folder = oldFolder;
        file.volume = spart;
        file.name = (char*)getFilename(oldpath);
        FS_ERROR error = spart->type->rename(&file, newFolder, getFilename(newpath));
        deviceManager_flushCaches(file.volume->disk);
        return (error);
    }
    else
    {
        // Create file at destination, write content of old file to it and remove old file
        FS_ERROR error = CE_GOOD;
        file_t* file1 = fopen(oldpath, "r");
        if (file1)
        {
            size_t size = file1->size;
            void* buffer = malloc(size, 0, "renameFile");
            fread(buffer, 1, size, file1);
            fclose(file1);

            file_t* file2 = fopen(newpath, "w");
            if (file2)
            {
                fwrite(buffer, 1, size, file2);

                if (!copy)
                    error = fmove(oldpath, 0, false); // Delete old file

                fclose(file2);
            }
            else
            {
                puts("\nError: file at new path could not be opened!");
                error = CE_FILENOTOPENED;
            }

            free(buffer);
        }
        else
            return CE_FILE_NOT_FOUND;
        return (error);
    }
}

char fgetc(file_t* file)
{
    char retVal;

    if (file->volume->type->fread(file, &retVal, 1) == CE_EOF)
        return (-1);

    return (retVal);
}

FS_ERROR fputc(char c, file_t* file)
{
    if (fwrite(&c, 1, 1, file) == 1)
        return CE_GOOD;
    return file->error;
}

char* fgets(char* dest, size_t num, file_t* file)
{
    for (size_t i = 0; i < num-1; i++)
    {
        dest[i] = fgetc(file);
        if (dest[i] == -1 || dest[i] == '\n')
        {
            dest[i] = 0;
            return (dest);
        }
    }
    dest[num-1] = 0;
    return (dest);
}

FS_ERROR fputs(const char* src, file_t* file)
{
    FS_ERROR retVal = CE_GOOD;
    for (; *src != 0 && retVal == CE_GOOD; src++)
    {
        retVal = fputc(*src, file);
    }
    return (retVal);
}

size_t fread(void* dest, size_t size, size_t count, file_t* file)
{
    file->error = file->volume->type->fread(file, dest, size * count);
    return (size * count);
}

size_t fwrite(const void* src, size_t size, size_t count, file_t* file)
{
    file->error = file->volume->type->fwrite(file, src, size * count);
    return (size * count);
}

FS_ERROR fflush(file_t* file)
{
    if (file->volume->type->fflush)
        return (file->volume->type->fflush(file));
    return CE_GOOD;
}

size_t ftell(file_t* file)
{
    return (file->seek);
}

FS_ERROR fseek(file_t* file, int32_t offset, SEEK_ORIGIN origin)
{
    return (file->volume->type->fseek(file, offset, origin));
}

FS_ERROR rewind(file_t* file)
{
    return (fseek(file, 0, SEEK_SET));
}

bool feof(file_t* file)
{
    return (file->EOF);
}

FS_ERROR ferror(file_t* file)
{
    if (file->error == CE_EOF)
        return CE_GOOD;
    return (file->error);
}

void clearerr(file_t* file)
{
    file->error = CE_GOOD;
}


//////////////////////
// Folder functions //
//////////////////////

static folder_t* getFolder_recursive(const char* path, folder_t* base)
{
    if (!base || !base->volume->type->folderAccess)
        return 0;

    const char* delim = strpbrk(path, "/|\\");
    if (delim == 0 || delim[1] == 0) // End of path
        return base;
    size_t sublength = delim ? (size_t)(delim - path) : strlen(path);

    char* name = malloc(sublength + 1, 0, "current folder name");
    memcpy(name, path, sublength);
    name[sublength] = 0;

    folder_t* current = 0;
    if (strcmp(name, "..") == 0) // Go one up
    {
        current = base->folder;
        free(name);
    }
    else // Go one down
    {
        fsnode_t* node = 0;
        for (dlelement_t* elem = base->nodes.head; elem; elem = elem->next) {
            node = elem->data;
            if (strcmp(name, node->name) == 0)
            {
                if (node->attributes & NODE_DIRECTORY)
                {
                    if (!node->folderData)
                        node->folderData = folderAccess_internal(name, base, base->volume, FOLDER_OPEN);
                    else
                        free(name);
                    break;
                }
                else // No directory
                {
                    free(name);
                    return 0;
                }
            }
            else
                node = 0;
        }

        if (!node) // Not found
        {
            free(name);
            return 0;
        }

        current = node->folderData;
    }
    return getFolder_recursive(delim + 1, current);
}


static folder_t* getFolder(const char* path, partition_t** part, bool isAbsolute, bool folderPath)
{
    if (strchr(path, ':') != 0) // Absolute path
    {
        *part = deviceManager_getPartition(path);
        if (!*part || !(*part)->type)
            return 0;
        path = strpbrk(path, "/|\\");
        if (path == 0 || (folderPath && path[0] != 0 && path[1] == 0))
            return 0;
        return getFolder_recursive(path + 1, (*part)->rootFolder);
    }
    if (isAbsolute)
        return 0;

    // Relative path - look in workingDir
    ipc_node_t* workingDirNode = ipc_getNode("~|workingDir");
    if (workingDirNode && workingDirNode->type == IPC_STRING)
    {
        folder_t* base = getFolder(workingDirNode->data.string, part, true, false);
        if (base)
        {
            folder_t* retval = getFolder_recursive(path, base);
            return retval;
        }
    }
    return 0;
}

static folder_t* folderAccess_internal(char* name, folder_t* parent, partition_t* volume, folderAccess_t mode)
{
    if (volume == 0)
        return 0;

    folder_t* folder = malloc(sizeof(folder_t), 0, "fsmgr-folder");
    folder->volume = volume;
    folder->folder = parent;
    folder->name = name;
    list_construct(&folder->nodes);

    if (folder->volume->type->folderAccess(folder, mode) != CE_GOOD)
    {   // cleanup
        list_destruct(&folder->nodes);
        free(folder->name);
        free(folder);
        return (0);
    }

    if (mode != FOLDER_OPEN)
        deviceManager_flushCaches(volume->disk);

    return (folder);
}

folder_t* folderAccess(const char* path, folderAccess_t mode)
{
    partition_t* volume = 0;
    folder_t* parent = getFolder(path, &volume, false, true);
    if ((!parent && getFilename(path) != path) || !volume || !volume->rootFolder)
        return 0;
    char* name = strdup(getFilename(path), "fsmgr-foldername");
    size_t nameLength = strlen(name);
    if (strchr("/|\\", name[nameLength - 1]) != 0) // Remove separator at the end, if present
        name[nameLength - 1] = '\0';
    if (memcmp(name, "..", 2) == 0)
    {   // Go one up
        free(name);
        if (!parent)
            return 0;
        return parent->folder;
    }
    return folderAccess_internal(name, parent, volume, mode);
}

void folderClose(folder_t* folder)
{
    if (folder == 0)
        return;

    folder->volume->type->folderClose(folder);
    list_destruct(&folder->nodes);
    free(folder->name);
    free(folder);
}


/////////////////////////
// Partition functions //
/////////////////////////

FS_ERROR formatPartition(const char* path, FS_t type, const char* name)
{
    partition_t* part = deviceManager_getPartition(path);
    if (!part)
        return CE_NOT_PRESENT;
    uint64_t ptype = getFSType(type);
    part->subtype = ptype;
    part->type = (fileSystem_t*)(uintptr_t)(ptype>>32);
    strcpy(part->name, name);
    FS_ERROR err = part->type->pformat(part);

    deviceManager_flushCaches(part->disk);
    return err;
}

FS_ERROR analyzePartition(partition_t* part)
{
    FS_ERROR error = CE_UNSUPPORTED_FS;

    part->rootFolder = malloc(sizeof(folder_t), 0, "part->rootFolder");
    part->rootFolder->data = 0;
    part->rootFolder->folder = 0;
    part->rootFolder->name = 0;
    part->rootFolder->volume = part;
    list_construct(&part->rootFolder->nodes);

    // Determine type of the partition:
    uint8_t buffer[512];
    singleSectorRead(part->start, buffer, part->disk);

    // Is it a BPB? -> FAT
    BPBbase_t* BPB = (BPBbase_t*)buffer;
    if (BPB->FATcount > 0 && BPB->bytesPerSector%512 == 0 && BPB->bytesPerSector != 0)
    {
        part->type = &FAT;
    }
    else // We only know FAT at the moment
    {
        part->type = 0;
        return (error);
    }

    if (part->type->pinstall)
        error = part->type->pinstall(part);

    if (error == CE_GOOD)
    {
        part->mount = true;

        if (part->type->folderAccess)
            return part->type->folderAccess(part->rootFolder, FOLDER_OPEN);
    }

    return (error);
}

void freePartition(partition_t* part)
{
    if (part->type)
    {
        if (part->type->folderClose && part->rootFolder)
            part->type->folderClose(part->rootFolder);
        part->type->pfree(part);
    }
    list_destruct(&part->rootFolder->nodes);
    free(part->rootFolder);
}


/////////////
// Helpers //
/////////////

void fsmanager_cleanup(task_t* task)
{
    if (task->files)
    {
        for (dlelement_t* e = task->files->head; e != 0; e = e->next)
        {
            file_t* file = e->data;
            fclose_task(file, task); // fclose_task deletes the list as soon as it is empty
        }
    }
    if (task->folders)
    {
        for (dlelement_t* e = task->folders->head; e != 0; e = e->next)
        {
            folder_t* folder = e->data;
            folderClose(folder);
        }
        list_free(task->folders);
    }
}

static uint64_t getFSType(FS_t type) // BIT 0-31: subtype, BIT 32-63: fileSystem_t
{
    switch (type)
    {
        case FS_FAT12: case FS_FAT16: case FS_FAT32:
            return (((uint64_t)(uintptr_t)&FAT << 32) | type);
        case FS_RAMFS:
            return (((uint64_t)(uintptr_t)&RAMFS << 32) | type);
    }
    return (0);
}


///////////////
// Debugging //
///////////////

static void indentNewLine(uint8_t indent)
{
    putch('\n');
    for (uint8_t i = 0; i < indent; i++)
        puts("    ");
}

static void logSubtree(folder_t* folder, uint8_t indent)
{
    if (!folder)
        return;

    indentNewLine(indent);
    puts("collapsed: ");
    textColor(DATA);
    bool first = true;
    for (dlelement_t* elem = folder->nodes.head; elem; elem = elem->next)
    {
        fsnode_t* node = elem->data;
        if (node->attributes & NODE_DIRECTORY && !node->folderData)
        {
            if (!first)
                puts(", ");
            else
                first = false;
            puts(node->name);
        }
    }
    textColor(TEXT);

    indentNewLine(indent);
    puts("expanded:");
    for (dlelement_t* elem = folder->nodes.head; elem; elem = elem->next)
    {
        fsnode_t* node = elem->data;
        if (node->attributes & NODE_DIRECTORY && node->folderData)
        {
            indentNewLine(indent);
            textColor(DATA);
            puts(node->name);
            textColor(TEXT);
            logSubtree(node->folderData, indent+1);
        }
    }
}

extern array(disk_t*) disks;
static void logPartition(size_t i, size_t j)
{
    textColor(IMPORTANT);
    printf("\nPartition %u:%u:|", i+1, j);
    textColor(TEXT);
    logSubtree(disks.data[i]->partition[j]->rootFolder, 0);
}

void fsmanager_log(void)
{
    textColor(HEADLINE);
    puts("\n\nStatus of FS-Manager:\n");
    for (size_t i = 0; i < disks.size; i++)
        if (disks.data[i])
            for (size_t j = 0; j < PARTITIONARRAYSIZE; j++)
                if (disks.data[i]->partition[j])
                    logPartition(i, j);
    textColor(TEXT);
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
