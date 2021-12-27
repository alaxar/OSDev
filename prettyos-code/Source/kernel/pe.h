#ifndef PE_H
#define PE_H

#include "filesystem/fsmanager.h"
#include "paging.h"


bool pe_checkFilename(const char* filename);
bool pe_checkFileformat(file_t* file);
void* pe_prepareExecution(const void* file, size_t size, pageDirectory_t* pd);


#endif
