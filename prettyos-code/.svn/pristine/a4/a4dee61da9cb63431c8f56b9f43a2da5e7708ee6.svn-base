#ifndef ELF_H
#define ELF_H

#include "filesystem/fsmanager.h"
#include "paging.h"


bool  elf_checkFilename(const char* filename);
bool  elf_checkFileformat(file_t* file);
void* elf_prepareExecution(const void* file, size_t size, pageDirectory_t* pd);


#endif
