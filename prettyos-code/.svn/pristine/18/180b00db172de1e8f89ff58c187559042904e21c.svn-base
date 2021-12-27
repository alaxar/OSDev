#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include "filesystem/fsmanager.h"
#include "paging.h"


typedef struct
{
    // Checks if the filename indicates that this file is an executable of this type
    bool  (*checkFilename)(const char*);
    // Checks if the file is of the correct format
    bool  (*checkFileformat)(file_t*);
    // Loads executable into given PD, prepared to be executed
    void* (*prepareExecution)(const void*, size_t, pageDirectory_t*); // file content, length, page directory. Returns entry point
} filetype_t;


FS_ERROR executeFile(const char* path, size_t argc, const char* argv [], bool ownConsole);


#endif
