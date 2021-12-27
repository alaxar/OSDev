#ifndef RAMDISK_H
#define RAMDISK_H

#include "filesystem/fsmanager.h"


typedef struct {
    fsnode_t node;
    void*    data;
    size_t   allocated_size;
} ramdisk_node_t;


void ramdisk_install(void); // Installs the initial ramdisk
bool ramdisk_loadShell(void); // Loads SHELL.ELF from ramdisk


file_t*  ramdisk_openFile(ramdisk_node_t* node, const char* path, const char* mode);
FS_ERROR ramdisk_fclose(file_t* file);
FS_ERROR ramdisk_fseek(file_t* file, int32_t offset, SEEK_ORIGIN origin);
FS_ERROR ramdisk_fread(file_t* file, void* destination, size_t size);
FS_ERROR ramdisk_fwrite(file_t* file, const void* source, size_t size);
FS_ERROR ramdisk_fflush(file_t* file);
FS_ERROR ramdisk_folderAccess(folder_t* folder, folderAccess_t mode);


#endif
