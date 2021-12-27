#ifndef HDD_H
#define HDD_H

#include "os.h"
#include "devicemanager.h"


void hdd_install(void);

FS_ERROR hdd_writeSectors(uint32_t sector, const void* buf, uint32_t count, disk_t* device);
FS_ERROR hdd_readSectors(uint32_t sector, void* buf, uint32_t count, disk_t* device);


#endif