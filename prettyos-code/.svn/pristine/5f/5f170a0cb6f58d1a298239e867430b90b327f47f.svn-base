/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "executable.h"
#include "util/util.h"
#include "memory.h"
#include "kheap.h"
#include "tasking/task.h"
#include "video/console.h"
#include "elf.h"
#include "pe.h"

enum FILETYPES {FT_ELF, FT_PE, FT_END};

static filetype_t filetypes[FT_END] =
{
    {&elf_checkFilename, &elf_checkFileformat, &elf_prepareExecution}, // ELF
    {&pe_checkFilename,  &pe_checkFileformat,  &pe_prepareExecution},  // PE
};


FS_ERROR executeFile(const char* path, size_t argc, const char* argv[], bool ownConsole)
{
    // Open file
    file_t* file = fopen(path, "r");
    if (file == 0) // File not found
    {
        return (CE_FILE_NOT_FOUND);
    }

    // Find out fileformat
    size_t i = 0;
    for (; i < FT_END; i++) // Check name and content of the file
    {
        if (filetypes[i].checkFilename != 0 && filetypes[i].checkFilename(path))
        {
            rewind(file);
            if (filetypes[i].checkFileformat != 0 && filetypes[i].checkFileformat(file))
            {
                break; // found
            }
        }
    }

    if (i == FT_END) // Not found, now do not look at filename, just content
    {
        for (i = 0; i < FT_END; i++)
        {
            rewind(file);
            if (filetypes[i].checkFileformat != 0 && filetypes[i].checkFileformat(file))
            {
                break; // found
            }
        }
    }

    if (i == FT_END)
    {
        fclose(file);
        printf("The file has an unknown type so it cannot be executed.");
        return (CE_BAD_FILE);
    }

    // Now execute
    size_t size = file->size;
    void* buffer = malloc(size, 0, "executeFile");
    rewind(file);
    fread(buffer, 1, size, file);
    if (ferror(file) != CE_GOOD)
    {
        free(buffer);
        printf("Reading the file failed (%u).", ferror(file));
        return ferror(file);
    }
    fclose(file);

    if (filetypes[i].prepareExecution != 0)
    {
        // Create page directory.
        pageDirectory_t* pd = paging_createPageDirectory();

        // Prepare executable. Load it into memory.
        void* entry = filetypes[i].prepareExecution(buffer, size, pd);
        if (entry == 0)
        {
            paging_destroyPageDirectory(pd);
            return (CE_BAD_FILE);
        }

        char** nnArgv = 0;
        // Copy argv to kernel PD (intermediate)
        if (argc != 0)
        {
            char* nArgv[argc];
            for (size_t index = 0; index < argc; index++)
            {
                nArgv[index] = strdup(argv[index], "temporary argv");
            }

            // Copy nArgv to user PD
            paging_alloc(pd, (void*)USER_DATA_BUFFER, (uintptr_t)USER_HEAP_START - (uintptr_t)USER_DATA_BUFFER, (MEMFLAGS_t)(MEM_USER | MEM_WRITE)); // Allocate space in user PD (Pages between heap and dataBuffer)
            cli();
            paging_switch(pd); // Switch to user PD
            nnArgv = (void*)USER_DATA_BUFFER; // argv buffer
            void* addr = nnArgv + sizeof(char*)*argc; // argv* strings stored after argv array
            for (size_t index = 0; index < argc; index++)
            {
                size_t argsize = strlen(nArgv[index]) + 1;
                nnArgv[index] = addr;
                memcpy(nnArgv[index], nArgv[index], argsize);
                addr += argsize;
            }
            paging_switch(currentTask->pageDirectory); // Switch back to old PD
            sti();

            // Free nArgv (allocated in kernelPD)
            for (size_t index = 0; index < argc; index++)
            {
                free(nArgv[index]);
            }
        }

        // Execute the task.
        if (ownConsole)
            scheduler_insertTask(create_cprocess(pd, entry, 3, argc, nnArgv, path));
        else
            scheduler_insertTask(create_process( pd, entry, 3, argc, nnArgv, path));
    }
    else
    {
        free(buffer);
        printf("Executing the file failed");
        return (CE_BAD_FILE);
    }

    free(buffer);
    return (CE_GOOD);
}

/*
* Copyright (c) 2011-2015 The PrettyOS Project. All rights reserved.
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
