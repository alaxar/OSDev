/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

/*
*  This code is based on example code for the PIC18F4550 given in the book:
*  Jan Axelson, "USB Mass Storage Device" (2006), web site: www.Lvr.com
*
*  The copyright, page ii, tells:
*  "No part of the book, except the program code, may be reproduced or transmitted in any form by any means
*  without the written permission of the publisher. The program code may be stored and executed in a computer system
*  and may be incorporated into computer programs developed by the reader."
*
*  We are adapting this sourcecode to the needs of PrettyOS.
*/

#include "fat.h"
#include "util/util.h"
#include "storage/devicemanager.h"
#include "kheap.h"
#include "video/console.h"
#include "time.h"
#include "serial.h"


enum // FAT subtypes
{
    FAT12, FAT16, FAT32,
    _SUBTYPE_COUNT
};

static const struct // Stores subtype-dependant constants
{
    const uint32_t end;
    const uint32_t last;
    const uint32_t fail;
} clusterVal[_SUBTYPE_COUNT] = {
    {.end = 0xFF7,      .last = 0xFF8,      .fail = 0xFFFF},
    {.end = 0xFFFE,     .last = 0xFFF8,     .fail = 0xFFFF},
    {.end = 0x0FFFFFF7, .last = 0x0FFFFFF8, .fail = 0x0FFFFFFF}
};


static FAT_dirEntry_t* FAT_getDirEntry(FAT_handle_t* handle);
static FS_ERROR FAT_writeDirEntry(FAT_partition_t* volume, uint32_t dirCluster, uint32_t curEntry);


////////////////////////////////
// cluster, clusterchain, fat //
////////////////////////////////

static uint32_t FAT_cluster2sector(FAT_partition_t* volume, uint32_t cluster)
{
    uint32_t sector;
    if (cluster <= 1) // root dir
    {
        if (volume->part->subtype == FS_FAT32) // In FAT32, there is no separate ROOT region. It is as well stored in DATA region
            sector = volume->dataLBA + cluster * volume->SecPerCluster;
        else
            sector = volume->root + cluster * volume->SecPerCluster;
    }
    else // data area
    {
        sector = volume->dataLBA + (cluster-2) * volume->SecPerCluster;
    }

  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_cluster2sector <<<<<    cluster: %u  sector %u", cluster, sector);
  #endif

    return (sector);
}

static uint32_t FAT_getFullClusterNumber(FAT_dirEntry_t* entry)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_getFullClusterNumber <<<<<");
  #endif

    uint32_t LongClusterNr = entry->FstClusHI;
    LongClusterNr = LongClusterNr << 16;
    LongClusterNr |= (uint32_t)entry->FstClusLO;
    return (LongClusterNr);
}

static uint32_t FAT_fatRead(FAT_partition_t* volume, uint32_t currCluster)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fatRead <<<<<");
  #endif

    uint8_t  q = 0;
    uint32_t posFAT; // position (byte) in FAT

    switch (volume->part->subtype)
    {
        case FS_FAT32:
            posFAT = currCluster * 4;
            break;
        case FS_FAT12:
            posFAT = currCluster * 3;
            q = posFAT & 1; // odd/even
            posFAT = posFAT >> 1;
            break;
        case FS_FAT16:
        default:
            posFAT = currCluster * 2;
            break;
    }

    uint32_t sector = volume->fat + (posFAT / volume->part->disk->sectorSize);

    uint8_t fat_buffer[volume->part->disk->sectorSize];

    if (singleSectorRead(sector, fat_buffer, volume->part->disk) != CE_GOOD)
        return (clusterVal[volume->type].fail);

    posFAT &= volume->part->disk->sectorSize - 1;
    uint32_t c = 0;

    switch (volume->part->subtype)
    {
        case FS_FAT32:
            c = MemoryReadLong(fat_buffer, posFAT);
            break;
        case FS_FAT16:
            c = MemoryReadWord(fat_buffer, posFAT);
            break;
        case FS_FAT12:
            c = MemoryReadByte(fat_buffer, posFAT);
            if (q)
                c >>= 4;
            posFAT = (posFAT + 1) & (volume->part->disk->sectorSize - 1);
            if (posFAT == 0)
            {
                if (singleSectorRead(sector + 1, fat_buffer, volume->part->disk) != CE_GOOD)
                    return (clusterVal[volume->type].fail);
            }
            uint32_t d = MemoryReadByte(fat_buffer, posFAT);
            if (q)
                c += d << 4;
            else
                c += (d & 0x0F) << 8;
            break;
    }

    return (min(c, clusterVal[volume->type].last));
}

static uint32_t FAT_fatWrite(FAT_partition_t* volume, uint32_t currCluster, uint32_t value)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fatWrite <<<<<");
  #endif

    uint32_t posFAT; // position (byte) in FAT
    uint8_t q = 0;
    switch (volume->part->subtype)
    {
        case FS_FAT32:
            posFAT = currCluster * 4;
            break;
        case FS_FAT12:
            posFAT = currCluster * 3;
            q = posFAT & 1; // odd/even
            posFAT = posFAT >> 1;
            break;
        case FS_FAT16:
        default:
            posFAT = currCluster * 2;
            break;
    }

    uint32_t sector = volume->fat + posFAT / volume->part->disk->sectorSize;
    posFAT &= volume->part->disk->sectorSize - 1;

    uint8_t fat_buffer[volume->part->disk->sectorSize];
    if (singleSectorRead(sector, fat_buffer, volume->part->disk) != CE_GOOD)
    {
        return (clusterVal[volume->type].fail);
    }

    switch (volume->part->subtype)
    {
        case FS_FAT32:
            *((uint32_t*)(fat_buffer + posFAT)) = value & 0x0FFFFFFF;
            break;
        case FS_FAT16:
            *((uint16_t*)(fat_buffer + posFAT)) = value;
            break;
        case FS_FAT12:
        {
            uint8_t c;
            if (q)
                c = ((value & 0x0F) << 4) | (MemoryReadByte(fat_buffer, posFAT) & 0x0F);
            else
                c = (value & 0xFF);

            MemoryWriteByte(fat_buffer, posFAT, c);

            posFAT = (posFAT + 1) & (volume->part->disk->sectorSize - 1);
            if (posFAT == 0)
            {
                if (singleSectorWrite(sector, fat_buffer, volume->part->disk) != CE_GOOD) // update FAT
                    return (clusterVal[volume->type].fail);
                sector++;
                if (singleSectorRead(sector, fat_buffer, volume->part->disk)  != CE_GOOD) // next sector
                    return (clusterVal[volume->type].fail);
            }
            c = MemoryReadByte(fat_buffer, posFAT); // second byte of the table entry
            if (q)
                c = (value >> 4);
            else
                c = ((value >> 8) & 0x0F) | (c & 0xF0);
            MemoryWriteByte(fat_buffer, posFAT, c);
            break;
        }
    }
    if (singleSectorWrite(sector, fat_buffer, volume->part->disk) != CE_GOOD)
        return (clusterVal[volume->type].fail);
    return (0);
}

static FS_ERROR FAT_eraseCluster(FAT_partition_t* volume, uint32_t cluster)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_eraseCluster <<<<<");
  #endif

    uint32_t SectorAddress = FAT_cluster2sector(volume, cluster);

    memset(volume->buffer, 0, volume->part->disk->sectorSize);

    volume->part->disk->accessRemaining += volume->SecPerCluster;
    for (uint16_t i = 0; i<volume->SecPerCluster; i++)
    {
        if (sectorWrite(SectorAddress++, volume->buffer, volume->part->disk) != CE_GOOD)
        {
            volume->part->disk->accessRemaining -= i;
            return (CE_WRITE_ERROR);
        }
    }
    return (CE_GOOD);
}

static bool FAT_eraseClusterChain(uint32_t cluster, FAT_partition_t* volume)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_eraseClusterChain <<<<<");
  #endif

    if (cluster <= 1)  // Cluster assigned can't be "0" and "1"
        return(true);

    for (;;)
    {
        uint32_t c = FAT_fatRead(volume, cluster);

        if (c == clusterVal[volume->type].fail)
            return(false);

        if (c <= 1)  // Cluster assigned can't be "0" and "1"
            return(true);

        if (c >= clusterVal[volume->type].last)
            return(true);

        if (FAT_fatWrite(volume, cluster, CLUSTER_EMPTY) == clusterVal[volume->type].fail)
            return(false);

        cluster = c;
    }
}

static FS_ERROR FAT_getNextCluster(FAT_partition_t* volume, uint32_t* currCluster, uint32_t n)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_getNextCluster <<<<<");
  #endif

    while(n-- > 0)
    {
        *currCluster = FAT_fatRead(volume, *currCluster);

        if (*currCluster == clusterVal[volume->type].fail)
            return (CE_BAD_SECTOR_READ);

        if (*currCluster >= clusterVal[volume->type].last)
            return (CE_FAT_EOF);

        if (*currCluster >= volume->maxcls)
            return (CE_INVALID_CLUSTER);
    }
    return (CE_GOOD);
}

static uint32_t FAT_findEmptyCluster(FAT_partition_t* volume, uint32_t startCluster)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_findEmptyCluster <<<<<");
  #endif

    uint32_t c = max(2, startCluster);

    FAT_fatRead(volume, c);

    uint32_t curcls = c;

    while (c)
    {
        uint32_t value = FAT_fatRead(volume, c);
        if (value == clusterVal[volume->type].fail)
            return (0);

        if (value == CLUSTER_EMPTY)
            return (c);

        c++;

        if (value == clusterVal[volume->type].end || c >= (volume->maxcls+2))
            c = 2;

        if (c == curcls)
            return (0);
    }

    return (c);
}

static FS_ERROR FAT_allocateNewCluster(FAT_partition_t* volume, uint32_t* currCluster, bool clear)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_allocateNewCluster <<<<<");
  #endif

    uint32_t c = FAT_findEmptyCluster(volume, *currCluster);
    if (c == 0)
        return (CE_DISK_FULL);

    FAT_fatWrite(volume, c, clusterVal[volume->type].last);

    FAT_fatWrite(volume, *currCluster, c);
    *currCluster = c;

    if (clear)
        return (FAT_eraseCluster(volume, c));

    return (CE_GOOD);
}

static FS_ERROR FAT_createHeadCluster(FAT_partition_t* volume, uint32_t* cluster)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_createHeadCluster <<<<<");
  #endif

    *cluster = FAT_findEmptyCluster(volume, *cluster);

    if (*cluster == 0)
        return (CE_DISK_FULL);

    if (FAT_fatWrite(volume, *cluster, clusterVal[volume->type].last) == clusterVal[volume->type].fail)
        return (CE_WRITE_ERROR);

    return (FAT_eraseCluster(volume, *cluster));
}

static FS_ERROR FAT_createFirstCluster(FAT_handle_t* handle)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_createFirstCluster <<<<<");
  #endif

    uint32_t cluster = 0;

    FS_ERROR error = FAT_createHeadCluster(handle->volume, &cluster);

    if (error == CE_GOOD)
    {
        FAT_dirEntry_t* entry = FAT_getDirEntry(handle);
        entry->FstClusHI      = (cluster & 0x0FFF0000) >> 16; // only 28 bits in FAT32
        entry->FstClusLO      = (cluster & 0x0000FFFF);
        handle->firstCluster  = cluster;

        return (FAT_writeDirEntry(handle->volume, handle->dirCluster, handle->entry));
    }

    return (error);
}


///////////////
// directory //
///////////////

static bool FAT_validateChars(char* FileName)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_validateChars <<<<<");
  #endif

    bool radix = false;
    for (uint16_t i = 0; FileName[i] != 0; i++)
    {
        if ((FileName[i] <= 0x20 &&  FileName[i] != 0x05) ||
             FileName[i] == '*'  ||  FileName[i] == '?'   ||
             FileName[i] == '"'  ||  FileName[i] == '+'   ||
             FileName[i] == ','  ||  FileName[i] == '/'   ||
             FileName[i] == ':'  ||  FileName[i] == ';'   ||
             FileName[i] == '<'  ||  FileName[i] == '='   ||
             FileName[i] == '>'  ||  FileName[i] == '['   ||
             FileName[i] == '\\' ||  FileName[i] == ']'   ||
             FileName[i] == '|'  || (FileName[i] == '.'  && radix))
        {
            return (false);
        }

        if (FileName[i] == '.')
            radix = true;

        FileName[i] = toUpper(FileName[i]); // Convert lower-case to upper-case
    }
    return (true);
}

static bool FAT_FormatFileName(const char* fileName, char* dest, bool dir)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_FormatFileName <<<<<");
  #endif

    memset(dest, ' ', 11);

    if (fileName[0] == '.' || fileName[0] == 0)
        return (false);

    char szName[13];
    size_t filenamesize = strlen(fileName);

    if (filenamesize <= FILE_NAME_SIZE+1)
        memcpy(szName, fileName, filenamesize+1);
    else
        return (false); //long file name

    if (!FAT_validateChars(szName))
        return (false);

    char* pExt = strchr(szName, '.');

    if (pExt != 0)
    {
        *pExt = 0;
        pExt++;

        if (dir && (strlen(pExt) > 3 || strlen(szName) > 8)) // No 8.3-Format
            return (false);
    }

    size_t szNameSize = strlen(szName);
    if (szNameSize > (dir ? FILE_NAME_SIZE : DIR_NAMESIZE))
        return (false);

    memcpy(dest, szName, szNameSize); // Do not copy 0

    if (pExt)
        strncpyandfill(dest + DIR_NAMESIZE, pExt, DIR_EXTENSION, ' ');

    return (true);
}

static void FAT_updateTimeStamp(FAT_dirEntry_t* entry)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_updateTimeStamp <<<<<");
  #endif

    tm_t time;
    cmosTime(&time);
    entry->WrtDate = ((time.century*100U + time.year- 1980U) << 9) | (time.month << 5) | time.dayofmonth;
    entry->WrtTime = (time.hour << 11) | (time.minute << 5) | (time.second/2U);
}

static FAT_dirEntry_t* FAT_cacheDirEntry(FAT_handle_t* handle, bool ForceRead)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_cacheDirEntry <<<<< ForceRead: %u", ForceRead);
  #endif

    FAT_partition_t* volume       = handle->volume;
    uint32_t DirectoriesPerSector = volume->part->disk->sectorSize/sizeof(FAT_dirEntry_t);
    uint32_t offset2              = handle->entry/DirectoriesPerSector;

    if (volume->part->subtype == FS_FAT32 || handle->parent->handle.firstCluster != 0) // FAT32 or no root dir
        offset2 %= volume->SecPerCluster;

    uint32_t currCluster = handle->dirCluster;

    if (ForceRead || (handle->entry & MASK_MAX_FILE_ENTRY_LIMIT_BITS) == 0)
    {
        // do we need to load the next cluster?
        if ((offset2 == 0 && handle->entry >= DirectoriesPerSector) || ForceRead)
        {
            if (handle->parent->handle.firstCluster == 0) // FAT12/16 root directory
            {
                currCluster = 0;
            }
            else
            {
                uint32_t NumberofCluster = 1;
                if (ForceRead)
                    NumberofCluster = handle->entry / (DirectoriesPerSector * volume->SecPerCluster);
                FAT_getNextCluster(volume, &currCluster, NumberofCluster);
            }
        }

        if (currCluster < clusterVal[volume->type].last)
        {
            handle->dirCluster = currCluster;
            uint32_t sector = FAT_cluster2sector(volume, currCluster);

            FS_ERROR error = singleSectorRead(sector + offset2, volume->buffer, volume->part->disk);

            if (error != CE_GOOD)
                return (0);

            if (ForceRead)
                return (FAT_dirEntry_t*)volume->buffer + (handle->entry%DirectoriesPerSector);

            return ((FAT_dirEntry_t*)volume->buffer);
        } // END: a valid cluster is found

        return (0); // invalid cluster number
    }

    return ((FAT_dirEntry_t*)volume->buffer + (handle->entry%DirectoriesPerSector));
}

bool FAT_findEmptyDirEntry(FAT_handle_t* handle)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_findEmptyDirEntry <<<<<");
  #endif

    handle->dirCluster = handle->parent->handle.firstCluster;
    FAT_dirEntry_t* entry = FAT_cacheDirEntry(handle, true);

    if (entry == 0)
    {
        return(false); // Failure
    }

    uint8_t a = (uint8_t)' ';
    for(;;)
    {
        uint8_t amountfound = 0;
        uint32_t bHandle = handle->entry;

        do
        {
            entry = FAT_cacheDirEntry(handle, true);
            if (entry != 0)
            {
                a = (uint8_t)entry->Name[0];
            }
            handle->entry++;
        } while ((a == DIR_DEL || a == DIR_EMPTY) && (entry != 0) &&  (++amountfound < 1));

        if (entry == 0)
        {
            uint32_t b = handle->dirCluster;
            if (b == handle->parent->handle.firstCluster)
            {
                if (handle->volume->part->subtype != FS_FAT32)
                    return(false);

                handle->firstCluster = b;

                if (FAT_allocateNewCluster(handle->volume, &handle->firstCluster, true) == CE_DISK_FULL)
                    return(false);

                handle->entry = bHandle;
                return(true);
            }

            handle->firstCluster = b;
            if (FAT_allocateNewCluster(handle->volume, &handle->firstCluster, true) == CE_DISK_FULL)
                return(false);

            handle->entry = bHandle;
            return(true);
        }

        if (amountfound == 1)
        {
            handle->entry = bHandle;
            return(true);
        }
    }
}

static FAT_dirEntry_t* FAT_getDirEntry(FAT_handle_t* handle)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_getDirEntry <<<<<");
  #endif

    handle->dirCluster = handle->parent->handle.firstCluster;
    FAT_dirEntry_t* entry = FAT_cacheDirEntry(handle, true);

    if ((entry == 0) || ((uint8_t)entry->Name[0] == DIR_EMPTY) || ((uint8_t)entry->Name[0] == DIR_DEL))
        return (0);

    while ((entry != 0) && (entry->Attr == ATTR_LONG_NAME))
    {
        handle->entry++;
        entry = FAT_cacheDirEntry(handle, false);
    }

    return (entry);
}

static FS_ERROR FAT_fillDirEntry(FAT_handle_t* handle, uint32_t* size, char *name, uint8_t mode)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fillDirEntry <<<<<");
  #endif

    FAT_folder_t* folder = handle->parent;
    handle->dirCluster   = folder->handle.firstCluster;
    FAT_dirEntry_t* entry  = FAT_cacheDirEntry(handle, true);

    if (entry == 0)
        return (CE_BADCACHEREAD);

    strncpy(entry->Name, name, FILE_NAME_SIZE); // HACK, accesses dir->Name and dir->Extension
    if (mode == DIRECTORY)
    {
        entry->Attr = ATTR_DIRECTORY;
    }
    else
    {
        entry->Attr = ATTR_ARCHIVE;
    }

    entry->FileSize  =    0x0;     // file size in uint32_t
    entry->NTRes     =    0x00;    // nt reserved
    entry->FstClusHI =    0x0000;  // hiword of this entry's first cluster number
    entry->FstClusLO =    0x0000;  // loword of this entry's first cluster number

   // time info
    tm_t time;
    cmosTime(&time);

    // last access date
    entry->LstAccDate = 0; // Unused by PrettyOS

    // last create/update date & time
    entry->CrtDate = entry->WrtDate = ((time.century*100U + time.year- 1980U) << 9) | (time.month << 5) | time.dayofmonth;
    entry->CrtTime = entry->WrtTime = (time.hour << 11) | (time.minute << 5) | (time.second/2U);
    entry->CrtTimeTenth = 0x00;    // millisecond stamp

    if(size)
        *size  = entry->FileSize;

    handle->time        = entry->CrtTime;
    handle->date        = entry->CrtDate;
    handle->attributes  = entry->Attr;

    return (FAT_writeDirEntry(handle->volume, handle->dirCluster, handle->entry));
}

FS_ERROR FAT_createDirEntry(FAT_handle_t* handle, uint8_t mode, uint32_t* size)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_createDirEntry <<<<<");
  #endif

    char name[FILE_NAME_SIZE];
    handle->entry = 0;

    memcpy(name, handle->name, FILE_NAME_SIZE);

    if (FAT_findEmptyDirEntry(handle))
    {
        FS_ERROR error = FAT_fillDirEntry(handle, size, name, mode);
        if (error == CE_GOOD)
            return (FAT_createFirstCluster(handle));
        return (error);
    }
    return (CE_DIR_FULL);
}

static FS_ERROR FAT_writeDirEntry(FAT_partition_t* volume, uint32_t dirCluster, uint32_t curEntry)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_writeDirEntry <<<<<");
  #endif

    uint8_t offset2 = curEntry / (volume->part->disk->sectorSize/32);

    if (volume->part->subtype == FS_FAT32 || dirCluster != 0)
        offset2 = offset2 % (volume->SecPerCluster);

    return (singleSectorWrite(FAT_cluster2sector(volume, dirCluster) + offset2, volume->buffer, volume->part->disk));
}

static FS_ERROR FAT_writeDotEntries(FAT_partition_t* fpart, uint32_t dotAddress, uint32_t dotdotAddress, uint16_t date, uint16_t time)
{
    uint8_t buffer[fpart->part->disk->sectorSize];
    memset(buffer, 0x00, fpart->part->disk->sectorSize);

    FAT_dirEntry_t* entry = (FAT_dirEntry_t*)buffer;

    entry->Name[0] = '.';
    memset(entry->Name+1, 0x20, 10);
    entry->Attr  = ATTR_DIRECTORY;
    entry->NTRes = 0x00;
    entry->FstClusLO = (uint16_t)(dotAddress & 0x0000FFFF);        // Lower 16 bit address
    entry->FstClusHI = (uint16_t)((dotAddress & 0x0FFF0000)>> 16); // Higher 16 bit address. FAT32 uses only 28 bits. Mask even higher nibble.
    entry->FileSize = 0x00;

    // Times need to be the same as the times in the directory entry. Set dir date for uncontrolled clock source
    entry->CrtTimeTenth = 0x00;
    entry->CrtTime      = time;
    entry->CrtDate      = date;
    entry->LstAccDate   = 0; // Unused by PrettyOS
    entry->WrtTime      = time;
    entry->WrtDate      = date;

    entry++;
    memcpy(entry, buffer, sizeof(FAT_dirEntry_t));

    entry->Name[1] = '.';
    entry->FstClusLO = (uint16_t)(dotdotAddress & 0x0000FFFF);        // Low  16 bit address
    entry->FstClusHI = (uint16_t)((dotdotAddress & 0x0FFF0000)>> 16); // High 16 bit address. FAT32 uses only 28 bits. Mask even higher nibble.

    return (singleSectorWrite(FAT_cluster2sector (fpart, dotAddress), buffer, fpart->part->disk));
}

#ifdef _FAT_DIAGNOSIS_
static void FAT_showDirEntry(FAT_dirEntry_t* entry)
{
    char name[9];
    name[8] = 0;
    memcpy(name, entry->Name, 8);
    char extension[4];
    extension[3] = 0;
    memcpy(extension, entry->Extension, 3);

    serial_log(SER_LOG_FAT, "\nname.ext: %s.%s",   name, extension);
    serial_log(SER_LOG_FAT, "\nattrib.:  %yh",     entry->Attr);
    serial_log(SER_LOG_FAT, "\ncluster:  %u",      FAT_getFullClusterNumber(entry));
    serial_log(SER_LOG_FAT, "\nfilesize: %u byte", entry->FileSize);
}
#endif


///////////////////////
// Handle Operations //
///////////////////////

static uint8_t FAT_fillHandle(FAT_handle_t* handle, uint32_t* size)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fillHandle <<<<<");
  #endif

    FAT_dirEntry_t* entry;
    FAT_folder_t* folder = handle->parent;

    if ((handle->entry & MASK_MAX_FILE_ENTRY_LIMIT_BITS) == 0 && handle->entry != 0) // 4-bit mask because 16 root entries max per sector
    {
        handle->dirCluster = folder->handle.firstCluster;
        entry = FAT_cacheDirEntry(handle, true);

      #ifdef _FAT_DIAGNOSIS_
        FAT_showDirEntry(entry);
      #endif
    }
    else
        entry = FAT_cacheDirEntry(handle, false);

    if ((entry == 0) || ((uint8_t)entry->Name[0] == DIR_EMPTY))
        return (NO_MORE);

    if ((uint8_t)entry->Name[0] == DIR_DEL)
        return (NOT_FOUND);

    memcpy(handle->name, entry->Name, FILE_NAME_SIZE); // HACK: Access file name and extension

    handle->firstCluster = FAT_getFullClusterNumber(entry);
    handle->time         = entry->WrtTime;
    handle->date         = entry->WrtDate;
    handle->attributes   = entry->Attr;

    if (size)
        *size = entry->FileSize;

    return (FOUND);
}

static FS_ERROR FAT_searchHandle(FAT_handle_t* handle, const char nameTest[11], uint32_t* size)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_searchHandle <<<<<");
  #endif

    FS_ERROR error       = CE_FILE_NOT_FOUND;
    FAT_folder_t* folder = handle->parent;
    handle->dirCluster   = folder->handle.firstCluster;
    memset(handle->name, 0x20, FILE_NAME_SIZE);

  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\nfHandle (FAT_searchHandle): %d", handle->entry);
  #endif

    if (handle->entry == 0 || (handle->entry & MASK_MAX_FILE_ENTRY_LIMIT_BITS) != 0) // Maximum 16 entries possible
    {
        if (FAT_cacheDirEntry(handle, true) == 0)
            return (CE_BADCACHEREAD);
    }

    while (error != CE_GOOD) // Loop until you reach the end or find the file
    {
      #ifdef _FAT_DIAGNOSIS_
        serial_log(SER_LOG_FAT, "\n\nentry %u\n", handle->entry);
      #endif

        uint8_t state = FAT_fillHandle(handle, size);

        if (state == NO_MORE)
            break;

        if (state == FOUND)
        {
          #ifdef _FAT_DIAGNOSIS_
            serial_log(SER_LOG_FAT, "\n\nstate == FOUND");
          #endif

            uint16_t attrib = handle->attributes & ATTR_MASK;

            if ((attrib != ATTR_VOLUME) && ((attrib & ATTR_HIDDEN) != ATTR_HIDDEN))
            {
              #ifdef _FAT_DIAGNOSIS_
                serial_log(SER_LOG_FAT, "\n\nAn entry is found. Attributes OK for search");
              #endif

                if (strncicmp(handle->name, nameTest, 11) == 0)
                    error = CE_GOOD; // it's a match
            }//if
        }//if

        if (error != CE_GOOD)
            handle->entry++; // increment it no matter what happened
    } // while
    return (error);
}

static FS_ERROR FAT_lookForHandle(FAT_handle_t* tempHandle, FAT_partition_t* fpart, FAT_folder_t* parent, const char* name, uint32_t* size)
{
    if (!FAT_FormatFileName(name, tempHandle->name, false)) // must be 8+3 formatted first
    {
        return (CE_INVALID_FILENAME);
    }

    tempHandle->volume = fpart;
    tempHandle->parent = parent;
    tempHandle->attributes = ATTR_ARCHIVE;
    tempHandle->firstCluster = 0;
    tempHandle->entry = 0;
    tempHandle->dirCluster = parent->handle.firstCluster;

  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\r\tempHandle->dirCluster: %u", tempHandle->dirCluster);
  #endif

    char search_name[11]; // Need a copy, because tempHandle.name is changed during search
    memcpy(search_name, tempHandle->name, 11);

    return (FAT_searchHandle(tempHandle, search_name, size));
}

static FS_ERROR FAT_renameHandle(FAT_handle_t* handle, const char* fileName)
{
    if (handle == 0)
        return CE_FILENOTOPENED;

    // Check that there is not already a file of this name
    FAT_handle_t tempHandle;
    FS_ERROR error = FAT_lookForHandle(&tempHandle, handle->volume, handle->parent, fileName, 0);

    if (error == CE_FILE_NOT_FOUND)
    {
        // Find dir entry to modify
        FAT_dirEntry_t* entry = FAT_getDirEntry(handle);

        if (entry == 0)
            return (CE_BADCACHEREAD);

        // Change entry
        FAT_FormatFileName(fileName, entry->Name, false); // HACK, accesses entry->Name and entry->Extension
        return (FAT_writeDirEntry(handle->volume, handle->dirCluster, handle->entry));
    }

    if (error == CE_GOOD)
        return (CE_FILENAME_EXISTS);
    else
        return (error);
}

FS_ERROR FAT_eraseHandle(FAT_handle_t* handle, bool EraseClusters)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_eraseHandle <<<<<");
  #endif

    FAT_folder_t* folder = handle->parent;
    handle->dirCluster = folder->handle.firstCluster;

    FAT_dirEntry_t* entry = FAT_cacheDirEntry(handle, true);

    if (entry == 0)
        return (CE_BADCACHEREAD);

    if (((uint8_t)entry->Name[0] == DIR_EMPTY) || ((uint8_t)entry->Name[0] == DIR_DEL))
        return (CE_FILE_NOT_FOUND);

    entry->Name[0] = DIR_DEL;
    uint32_t cluster = FAT_getFullClusterNumber(entry);

    if (FAT_writeDirEntry(handle->volume, handle->dirCluster, handle->entry) != CE_GOOD)
        return (CE_ERASE_FAIL);

    if (EraseClusters && cluster != folder->handle.firstCluster)
        FAT_eraseClusterChain(cluster, handle->volume);

    return (CE_GOOD);
}


/////////////////////
// File Operations //
/////////////////////

static FS_ERROR FAT_fdopen(FAT_file_t* fileptr, bool write)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fdopen <<<<<");
  #endif

    partition_t* volume = fileptr->handle.volume->part;
    if (volume->mount == false)
    {
        return (CE_NOT_INIT);
    }

    FS_ERROR error = CE_GOOD;

    FAT_folder_t* folder = fileptr->file->folder->data;
    fileptr->handle.dirCluster = folder->handle.firstCluster;
    if (fileptr->handle.entry == 0)
    {
        if (FAT_cacheDirEntry(&fileptr->handle, true) == 0)
            error = CE_BADCACHEREAD;
    }
    else
    {
        if ((fileptr->handle.entry & 0xf) != 0)
        {
            if (FAT_cacheDirEntry(&fileptr->handle, true) == 0)
                error = CE_BADCACHEREAD;
        }
    }

    if (FAT_fillHandle(&fileptr->handle, &fileptr->file->size) != FOUND)
        return (CE_FILE_NOT_FOUND);

    fileptr->file->seek  = 0;
    fileptr->currCluster = fileptr->handle.firstCluster;
    fileptr->sec = 0;
    fileptr->pos = 0;

    fileptr->file->EOF = false;

    fileptr->file->write = write;
    fileptr->file->read  = !fileptr->file->write;

    if (fileptr->handle.attributes & ATTR_DIRECTORY)
        return (CE_FOPEN_ON_DIR);
    else
        return (error);
}

FS_ERROR FAT_fopen(file_t* file, bool create, bool overwrite)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fopen <<<<<");
#endif

    FAT_file_t* FATfile = malloc(sizeof(FAT_file_t), 0, "FAT_fopen-FATfile");
    file->data = FATfile;
    FATfile->file = file;
    FATfile->currCluster = 0;

    char name[11];
    FAT_FormatFileName(file->name, name, false);

    FS_ERROR error = FAT_lookForHandle(&FATfile->handle, file->volume->data, file->folder->data, file->name, &file->size);
    if (error == CE_INVALID_FILENAME)
        return (CE_INVALID_FILENAME);

    if (error == CE_GOOD)
    {
        if (overwrite || file->size == 0) // Should overwrite. TODO: Replace size==0 condition by something smarter
        {
            error = FAT_eraseHandle(&FATfile->handle, true);

            if (error == CE_GOOD)
            {
                error = FAT_createDirEntry(&FATfile->handle, 0, &FATfile->file->size);

                if (error == CE_GOOD)
                {
                    error = FAT_fdopen(FATfile, true);
                }
            }
        }
        else if (create) // Should not overwrite, create is allowed, so it is not 'r'-mode -> Append
        {
            error = FAT_fdopen(FATfile, true);
        }
        else
        {
            error = FAT_fdopen(FATfile, false);
        }
    }
    else
    {
        if (create)
        {
            memcpy(FATfile->handle.name, name, 11); // Name in file handle has been changed during search. Restore it.
            error = FAT_createDirEntry(&FATfile->handle, 0, &FATfile->file->size);

            if (error == CE_GOOD)
            {
                error = FAT_fdopen(FATfile, true);
            }
        }
        else
        {
            error = CE_FILE_NOT_FOUND;
        }
    }

    if (error != CE_GOOD)
        free(FATfile);

    return (error);
}

FS_ERROR FAT_fread(file_t* file, void* dest, size_t count)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fread <<<<<");
  #endif

    FAT_file_t* fatfile = file->data;
    FS_ERROR error      = CE_GOOD;
    partition_t* volume = fatfile->handle.volume->part;
    FAT_partition_t* fatpart = volume->data;
    uint16_t pos        = fatfile->pos;
    uint32_t seek       = file->seek;
    uint32_t size       = file->size;
    uint32_t sector     = FAT_cluster2sector(volume->data, fatfile->currCluster) + fatfile->sec;
    uint32_t sectors    = (size%512 == 0) ? size/512 : size/512+1; // Number of sectors to be read
    volume->disk->accessRemaining += sectors;

    while (error == CE_GOOD && count > 0)
    {
        if (seek == size)
        {
            error = CE_EOF;
            break;
        }

        if (pos == volume->disk->sectorSize)
        {
            pos = 0;
            fatfile->sec++;
            sector++;
            if (fatfile->sec == fatfile->handle.volume->SecPerCluster)
            {
                fatfile->sec = 0;
                error = FAT_getNextCluster(volume->data, &fatfile->currCluster, 1);
                sector = FAT_cluster2sector(volume->data, fatfile->currCluster);
            }
        } // END: load new sector

        if (error == CE_GOOD)
        {
            sectors--;
            if (sectorRead(sector, fatpart->buffer, volume->disk) != CE_GOOD)
            {
                error = CE_BAD_SECTOR_READ;
            }
        }

        if (error == CE_GOOD)
        {
            size_t bytes = min(count, volume->disk->sectorSize - pos);
            memcpy(dest, fatpart->buffer + pos, bytes);
            pos += bytes;
            dest += bytes;
            seek += bytes;
            count -= bytes;
        }
    } // while no error and more bytes to copy

    volume->disk->accessRemaining -= sectors; // Subtract sectors which has not been read

    fatfile->pos = pos;
    file->seek = seek;
    return (error);
}

FS_ERROR FAT_fseek(file_t* file, int32_t offset, SEEK_ORIGIN whence)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fseek<<<<<");
  #endif

    FAT_file_t* FATfile = file->data;
    FAT_partition_t* volume = FATfile->handle.volume;

    uint32_t pos;
    switch (whence)
    {
        case SEEK_CUR:
            pos = (uint32_t)((int32_t)file->seek + offset);
            break;
        case SEEK_END:
            pos = (uint32_t)((int32_t)file->size - offset);
            break;
        default:
            pos = (uint32_t)offset;
            break;
    }

    FATfile->currCluster = FATfile->handle.firstCluster;
    uint32_t temp =  file->size;

    if (pos > temp)
        return (CE_SEEK_ERROR);

    file->EOF = false;
    file->seek = pos;
    uint32_t numsector = pos / volume->part->disk->sectorSize;
    pos -= (numsector * volume->part->disk->sectorSize);
    FATfile->pos = pos;
    temp = numsector / volume->SecPerCluster;
    numsector -= (volume->SecPerCluster * temp);
    FATfile->sec = numsector;

    if (temp > 0)
    {
        FS_ERROR test = FAT_getNextCluster(FATfile->handle.volume, &FATfile->currCluster, temp);
        if (test != CE_GOOD)
        {
            if (test == CE_FAT_EOF)
            {
                if (file->write)
                {
                    FATfile->currCluster = FATfile->handle.firstCluster;

                    if (temp != 1)
                        test = FAT_getNextCluster(FATfile->handle.volume, &FATfile->currCluster, temp-1);

                    if (FAT_allocateNewCluster(volume, &FATfile->currCluster, false) != CE_GOOD)
                        return (CE_COULD_NOT_GET_CLUSTER);
                }
                else
                {
                    FATfile->currCluster = FATfile->handle.firstCluster;
                    test = FAT_getNextCluster(FATfile->handle.volume, &FATfile->currCluster, temp-1);
                    if (test != CE_GOOD)
                        return (CE_COULD_NOT_GET_CLUSTER);

                    FATfile->pos = volume->part->disk->sectorSize;
                    FATfile->sec = volume->SecPerCluster - 1;
                }
            }
            else
            {
                return (CE_SEEK_ERROR);
            }
        }
    }

    return (CE_GOOD);
}

FS_ERROR FAT_remove(file_t* file)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n FAT_remove: file: %X", file);
  #endif

    FAT_handle_t tempHandle;
    FS_ERROR error = FAT_lookForHandle(&tempHandle, file->volume->data, file->folder->data, file->name, 0);
    if (error != CE_GOOD)
        return (error);
    if (tempHandle.attributes & ATTR_DIRECTORY)
        return (CE_DELETE_DIR);
    return (FAT_eraseHandle(&tempHandle, true));
}

FS_ERROR FAT_rename(file_t* file, folder_t* newFolder, const char* newName)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n FAT_rename: file: %X, newFolder: %X, newName: %s", file, newFolder, newName);
  #endif

    FAT_handle_t tempHandle;

    FS_ERROR error = FAT_lookForHandle(&tempHandle, file->volume->data, file->folder->data, file->name, 0);
    if (error != CE_GOOD)
        return (error);

    return (FAT_renameHandle(&tempHandle, newName));
}

FS_ERROR FAT_fwrite(file_t* file, const void* src, size_t count)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fwrite <<<<<");
  #endif

    if (!file->write)
        return (CE_READONLY);
    if (!count)
        return (CE_GOOD);

    FAT_file_t* fatfile = file->data;
    partition_t* volume = file->volume;
    FAT_partition_t* fatpart = volume->data;
    uint16_t pos        = fatfile->pos;
    uint32_t seek       = file->seek;
    uint32_t sector     = FAT_cluster2sector(volume->data, fatfile->currCluster) + fatfile->sec;
    uint32_t sectors    = (count%512 == 0) ? count/512 : count/512+1; // Number of sectors to be written

    if (pos > 0 && singleSectorRead(sector, fatpart->buffer, volume->disk) != CE_GOOD)
        return(CE_BAD_SECTOR_READ);

    volume->disk->accessRemaining += sectors;
    uint32_t filesize   = file->size;
    FS_ERROR error      = CE_GOOD;

    while (error==CE_GOOD && count>0)
    {
        if (seek==filesize)
        {
            file->EOF = true;
        }

        if (pos == volume->disk->sectorSize)
        {
            pos = 0;
            fatfile->sec++;
            if (fatfile->sec == fatfile->handle.volume->SecPerCluster)
            {
                fatfile->sec = 0;

                if (file->EOF)
                {
                    error = FAT_allocateNewCluster(volume->data, &fatfile->currCluster, false);
                }
                else
                {
                    error = FAT_getNextCluster(volume->data, &fatfile->currCluster, 1);
                }
            }

            if (error == CE_GOOD)
            {
                sector = FAT_cluster2sector(volume->data, fatfile->currCluster);
                sector += fatfile->sec;
            }
        } //  load new sector

        if (error == CE_GOOD)
        {
            size_t bytes = min(count, volume->disk->sectorSize-pos);
            memcpy(fatpart->buffer + pos, src, bytes);
            pos += bytes;
            src += bytes;
            seek += bytes;
            count -= bytes;
            if (file->EOF)
            {
                filesize += bytes;
            }
            sectors--;
            error = sectorWrite(sector, fatpart->buffer, volume->disk);
        }
    } // while count

    volume->disk->accessRemaining -= sectors; // Subtract sectors that have not been written

    fatfile->pos = pos;      // save positon
    file->seek   = seek;     // save seek
    file->size   = filesize; // new filesize

    return (error);
}

FS_ERROR FAT_fclose(file_t* file)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_fclose <<<<<");
  #endif

    FAT_file_t* FATfile = file->data;
    FS_ERROR error      = CE_GOOD;

    if (file->write)
    {
        FAT_dirEntry_t* entry = FAT_getDirEntry(&FATfile->handle);

        if (entry == 0)
            return (CE_EOF);

        FAT_updateTimeStamp(entry);

        entry->FileSize = file->size;
        entry->Attr     = FATfile->handle.attributes;

        error = FAT_writeDirEntry(FATfile->handle.volume, FATfile->handle.dirCluster, FATfile->handle.entry);
    }

    free(FATfile);
    return (error);
}


////////////////////////
//  Folder Operations //
////////////////////////

static FS_ERROR FAT_createFolder(folder_t* folder)
{
    FAT_folder_t* fatFolder  = malloc(sizeof(FAT_folder_t), 0, "FAT_folder_t");
    fatFolder->folder        = folder;
    fatFolder->handle.parent = folder->folder->data;
    fatFolder->handle.volume = folder->volume->data;
    folder->data = fatFolder;

    if (FAT_FormatFileName(fatFolder->folder->name, fatFolder->handle.name, true) == false)
        return (CE_INVALID_FILENAME);

    // Create a directory entry
    FS_ERROR error = FAT_createDirEntry(&fatFolder->handle, DIRECTORY, 0);
    if (error != CE_GOOD)
        return (error);

    if (FAT_fatWrite(fatFolder->handle.volume, 0, 0))
        return (CE_WRITE_ERROR);

    uint32_t dotdot = fatFolder->handle.parent->handle.firstCluster;
    uint32_t dot    = fatFolder->handle.firstCluster;

    return (FAT_writeDotEntries(fatFolder->handle.volume, dot, dotdot, fatFolder->handle.date, fatFolder->handle.time));
}

static FS_ERROR FAT_openFolder(folder_t* folder)
{
    // Read directory content. Analyze it as a FAT_dirEntry_t array. Put all valid entries into the already created subfolder and files lists.

#ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_openFolder <<<<<");
#endif

    FAT_partition_t* fpart = folder->volume->data;

    FAT_folder_t* fatFolder = malloc(sizeof(FAT_folder_t), 0, "FAT_folder_t");
    fatFolder->folder = folder;
    if (folder->folder)
        fatFolder->handle.parent = folder->folder->data;
    else
        fatFolder->handle.parent = 0;
    fatFolder->handle.volume = fpart;
    folder->data = fatFolder;

    if (folder->folder == 0)
        fatFolder->handle.firstCluster = fpart->rootFolder.handle.firstCluster;
    else
    {
        FS_ERROR error = FAT_lookForHandle(&fatFolder->handle, fpart, fatFolder->handle.parent, folder->name, 0);
        // Check if its a directory
        if (error == CE_GOOD && !(fatFolder->handle.attributes & ATTR_DIRECTORY))
            error = CE_DIR_NOT_FOUND;

        if (error != CE_GOOD)
        {
            free(fatFolder);
            folder->data = 0;
            return error;
        }
    }

    FAT_handle_t handle;
    handle.dirCluster = fatFolder->handle.firstCluster;
    handle.volume = fpart;
    handle.parent = fatFolder;
    handle.entry = 0;

    while (true) // Loop until you reach the end or find the file
    {
      #ifdef _FAT_DIAGNOSIS_
        serial_log(SER_LOG_FAT, "\n\nhandle.entry %u\n", handle.entry);
      #endif

        FAT_dirEntry_t* entry;

        if ((handle.entry & MASK_MAX_FILE_ENTRY_LIMIT_BITS) == 0 || handle.entry == 0) // 4-bit mask because 16 root entries max per sector
        {
            handle.dirCluster = fatFolder->handle.firstCluster;
            entry = FAT_cacheDirEntry(&handle, true);

          #ifdef _FAT_DIAGNOSIS_
            FAT_showDirEntry(entry);
          #endif
        }
        else
            entry = FAT_cacheDirEntry(&handle, false);

        if (!entry)
            return CE_BADCACHEREAD;

        if (entry->Name[0] == DIR_EMPTY)
            break; // free from here on

        if ((uint8_t)entry->Name[0] != DIR_DEL &&            // Entry is not deleted
           (entry->Attr & ATTR_LONG_NAME) != ATTR_LONG_NAME) // Entry is not part of long file name (VFAT)
        {
            fsnode_t* node = malloc(sizeof(fsnode_t), 0, "fsnode");
            node->name = malloc(8+1+3+1, 0, "fsnode.name"); // Space for filename.ext (8+3)
            node->folderData = 0;

            // Filename
            size_t letters = 0;
            for (uint8_t j = 0; j < 8; j++)
            {
                if (entry->Name[j] != 0x20) // Empty space
                {
                    node->name[letters] = entry->Name[j];
                    letters++;
                }
            }
            if (!(entry->Attr & ATTR_VOLUME) && // No volume label
                 entry->Extension[0] != 0x20 && entry->Extension[1] != 0x20 && entry->Extension[2] != 0x20) // Has extension
            {
                node->name[letters] = '.';
                letters++;
            }

            for (uint8_t j = 0; j < 3; j++)
            {
                if (entry->Extension[j] != 0x20) // Empty space
                {
                    node->name[letters] = entry->Extension[j];
                    letters++;
                }
            }
            node->name[letters] = 0;

            if (strcmp(node->name, ".") == 0 || strcmp(node->name, "..") == 0) // Don't show . and ..
            {
                free(node->name);
                free(node);
            }
            else
            {
                if (!(entry->Attr & ATTR_DIRECTORY) && !(entry->Attr & ATTR_VOLUME))
                {
                    node->size = entry->FileSize;
                }

                node->attributes = 0;
                if (entry->Attr & ATTR_VOLUME)           node->attributes |= NODE_VOLUME;
                if (entry->Attr & ATTR_DIRECTORY)        node->attributes |= NODE_DIRECTORY;
                if (entry->Attr & ATTR_READ_ONLY)        node->attributes |= NODE_READONLY;
                if (entry->Attr & ATTR_HIDDEN)           node->attributes |= NODE_HIDDEN;
                if (entry->Attr & ATTR_SYSTEM)           node->attributes |= NODE_SYSTEM;
                if (entry->Attr & ATTR_ARCHIVE)          node->attributes |= NODE_ARCHIVE;

                list_append(&folder->nodes, node);
            }
        }
        handle.entry++; // increment it no matter what happened
    } // while

    return CE_GOOD;
}

static FS_ERROR FAT_deleteFolder(folder_t* folder)
{
    FAT_handle_t handle;
    FS_ERROR error = FAT_lookForHandle(&handle, folder->volume->data, folder->folder->data, folder->name, 0);

    if (error != CE_GOOD)
        return (error);
    else
        return (FAT_eraseHandle(&handle, true));
}

FS_ERROR FAT_folderAccess(folder_t* folder, folderAccess_t mode)
{
    folder->data = 0;
    switch (mode)
    {
        case FOLDER_OPEN:    return (FAT_openFolder(folder));
        case FOLDER_CREATE:  return (FAT_createFolder(folder));
        case FOLDER_DELETE:  return (FAT_deleteFolder(folder));
    }
    return (CE_INVALID_ARGUMENT);
}

void FAT_folderClose(folder_t* folder)
{
    for (dlelement_t* e = folder->nodes.head; e; e = e->next)
    {
        fsnode_t* node = e->data;
        free(node->name);
        free(node);
    }
    free(folder->data);
}


////////////////
//  Partition //
////////////////

static void FAT_fillBootsector(partition_t* part, uint8_t* sector)
{
    // Prepare some temp data used later
    tm_t time;
    cmosTime(&time);
    uint32_t compressedTime = (time.year << 24) | (time.month << 20) | (time.dayofmonth << 15) | (time.hour << 10) | (time.minute << 4) | time.second;

    FAT_partition_t* fpart = part->data;

    // Prepare boot sector
    memset(sector, 0, 512);

    // jump ahead 60 bytes to the boot code which begins at byte 0x3E in the BBP
    sector[0] = 0xEB; // jump instruction
    sector[1] = 0x3C; // 60 (bytes)
    sector[2] = 0x90; // no operation (NOP)

    // BIOS parameter block // numbers are in little-endian format
    strncpy((char*)sector+3, "MSWIN4.1", 8); // Windows does not pay any attention to this field. Some FAT drivers do.
    sector[0x0B] = BYTE1(part->disk->sectorSize); // Bytes per sector
    sector[0x0C] = BYTE2(part->disk->sectorSize);
    sector[0x0D] = fpart->SecPerCluster; // Sectors per cluster
    sector[0x0E] = BYTE1(fpart->reservedSectors); // Reserved sectors
    sector[0x0F] = BYTE2(fpart->reservedSectors);
    sector[0x10] = 2; // Number of FATs
    sector[0x11] = BYTE1(fpart->maxroot); // Maximum of root entries
    sector[0x12] = BYTE2(fpart->maxroot);
    sector[0x13] = sector[0x14] = 0; // Number of sectors. If 0, it means that the bigger 32-bit field at 0x20 is used.

    if (part->disk->type == &FLOPPYDISK || part->disk->type == &USB_MSD)
        sector[0x15] = 0xF0; // Media descriptor (0xF0: Floppy with 80 tracks and 18 sectors per track)
    else
        sector[0x15] = 0xF8; // Media descriptor (0xF8: Hard disk)

    if (part->subtype == FS_FAT32)
    {
        sector[0x16] = 0; // FAT size
        sector[0x17] = 0;
    }
    else
    {
        sector[0x16] = BYTE1(fpart->fatsize); // Number of sectors for FAT12 and 16
        sector[0x17] = BYTE2(fpart->fatsize);
    }

    sector[0x18] = BYTE1(part->disk->secPerTrack); // Sectors per track
    sector[0x19] = BYTE2(part->disk->secPerTrack);
    sector[0x1A] = BYTE1(part->disk->headCount); // Number of heads
    sector[0x1B] = BYTE2(part->disk->headCount);
    sector[0x1C] = BYTE1(part->start); // Hidden sectors
    sector[0x1D] = BYTE2(part->start);
    sector[0x1E] = BYTE3(part->start);
    sector[0x1F] = BYTE4(part->start);
    sector[0x20] = BYTE1(part->size); // Number of sectors
    sector[0x21] = BYTE2(part->size);
    sector[0x22] = BYTE3(part->size);
    sector[0x23] = BYTE4(part->size);

    // Extended Boot Record
    if (part->subtype == FS_FAT32)
    {
        sector[0x24] = BYTE1(fpart->fatsize); // FAT size
        sector[0x25] = BYTE2(fpart->fatsize);
        sector[0x26] = BYTE3(fpart->fatsize);
        sector[0x27] = BYTE4(fpart->fatsize);
        sector[0x28] = 0; // FAT flags
        sector[0x29] = 0;
        sector[0x2A] = 0; // FAT32 version // high: major version, low: is the minor version. FAT drivers should respect this field.
        sector[0x2B] = 0; // modern sticks use 0 0
        sector[0x2C] = BYTE1(2); // First cluster of root. Typically: 2 (only higher, if bad cluster)
        sector[0x2D] = BYTE2(2); // ...
        sector[0x2E] = BYTE3(2); // ...
        sector[0x2F] = BYTE4(2); // ...
        sector[0x30] = BYTE1(1); //  sector number of the FSInfo structure
        sector[0x31] = BYTE2(1); // ...
        sector[0x32] = BYTE1(6); // sector number of the copy of the bootsector // No value other than 6 is recommended!
        sector[0x33] = BYTE2(6); // ...
        memset(sector+0x34, 0, 12); // reserved // When the volume is formated these bytes should be zero.
        sector[0x40] = part->disk->BIOS_driveNum = 0x80; // // BIOS number of device. 0: FDD, 0x80 HDD
        sector[0x41] = 0; // reserved
        sector[0x42] = 0x29; // Extended boot signature, set to 0x29 if the 3 fields below are present!
        sector[0x43] = BYTE1(compressedTime); // Filesystem ID
        sector[0x44] = BYTE2(compressedTime);
        sector[0x45] = BYTE3(compressedTime);
        sector[0x46] = BYTE4(compressedTime);
        strncpyandfill((char*)sector+0x47, part->name, 11, ' '); // Name of filesystem
        strncpyandfill((char*)sector+0x36, "FAT32", 8, ' '); // FAT type
    }
    else // FAT12, FAT16
    {
        if (part->subtype == FS_FAT12)
            sector[0x24] = part->disk->BIOS_driveNum = 0; // BIOS number of device. 0: FDD, 0x80 HDD
        else
            sector[0x24] = part->disk->BIOS_driveNum = 0x80; // BIOS number of device. 0: FDD, 0x80 HDD
        sector[0x25] = 0; // Reserved
        sector[0x26] = 0x29; // Extended boot signature // must be 0x28 or 0x29
        sector[0x27] = BYTE1(compressedTime); // VolumeID 'Serial' number
        sector[0x28] = BYTE2(compressedTime);
        sector[0x29] = BYTE3(compressedTime);
        sector[0x2A] = BYTE4(compressedTime);
        strncpyandfill((char*)sector+0x2B, part->name, 11, ' '); // Name of filesystem
        if (part->subtype == FS_FAT12)
            strncpyandfill((char*)sector+0x36, "FAT12", 8, ' '); // FAT type
        else
            strncpyandfill((char*)sector+0x36, "FAT16", 8, ' '); // FAT type
    }

    // Boot signature
    sector[510] = 0x55;
    sector[511] = 0xAA;
}


typedef struct
{
    uint32_t size;
    uint16_t secperclus;
} secPerClusTable_t;

static const secPerClusTable_t DskTableFAT32[] = {
    { 66600,       1 }, // { 66600, 0 },       // disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error
    { 532480,      1 }, // disks up to  260 MB, .5k cluster
    { 16777216,    8 }, // disks up to    8 GB,  4k cluster
    { 33554432,   16 }, // disks up to   16 GB,  8k cluster
    { 67108864,   32 }, // disks up to   32 GB, 16k cluster
    { 0xFFFFFFFF, 64 }, // disks greater than 32 GB, 32k cluster
    { 0, 0 }            // disks greater than 8TB, 0 value for SecPerClusVal trips an error
};

static const secPerClusTable_t DskTableFAT16[] = {
    { 8400,      0 }, // disks up to 4.1 MB, the 0 value for SecPerClusVal trips an error
    { 32680,     2 }, // disks up to  16 MB,  1k cluster
    { 262144,    4 }, // disks up to 128 MB,  2k cluster
    { 524288,    8 }, // disks up to 256 MB,  4k cluster
    { 1048576,  16 }, // disks up to 512 MB,  8k cluster
    { 2097152,  32 }, // disks up to   1 GB, 16k cluster
    { 4194304,  64 }, // disks up to   2 GB, 32k cluster
    { 8388608, 128 }, // disks up to   4 GB, 64k cluster
    { 0, 0 }          // any disk greater than 2GB, 0 value for SecPerClusVal trips an error
};

static uint16_t lookupSecPerClus(const secPerClusTable_t* table, uint32_t sectors)
{
    for (; table->size; table++) {
        if (table->size > sectors)
            return table->secperclus;
    }
    return 0;
}

FS_ERROR FAT_format(partition_t* part)
{
    if (part->type != &FAT) // TODO: Handle case that partition was not formatted with FAT before
        return CE_UNSUPPORTED_FS;

    FAT_partition_t* fpart = part->data;

    // Initialize FAT_partition_t
    fpart->fatcopy = 2; // typical: 2
    if (part->subtype == FS_FAT12)
    {
        fpart->type = FAT12;
        fpart->reservedSectors = 1;
        fpart->maxroot = 224;
        fpart->SecPerCluster = 1;
        fpart->fatsize = 9;
    }
    else
    {
        if (part->subtype == FS_FAT16)
        {
            fpart->type = FAT16;
            fpart->reservedSectors = 1;
            fpart->maxroot = 512;
            fpart->SecPerCluster = lookupSecPerClus(DskTableFAT16, part->size);
        }
        else
        {
            fpart->type = FAT32;
            fpart->reservedSectors = 32;
            fpart->maxroot = 0;
            fpart->SecPerCluster = lookupSecPerClus(DskTableFAT32, part->size);
        }
        if (fpart->SecPerCluster == 0)
            return CE_BAD_FORMAT_PARAM;

        // Calculating FAT size according to specification
        uint32_t RootDirSectors = ((fpart->maxroot * 32) + (part->disk->sectorSize - 1)) / part->disk->sectorSize;
        uint32_t TmpVal1 = part->size - (fpart->reservedSectors + RootDirSectors);
        uint32_t TmpVal2 = (256 * fpart->SecPerCluster) + fpart->fatcopy;
        if (part->subtype == FS_FAT32)
            TmpVal2 = TmpVal2 / 2;
        fpart->fatsize = (TmpVal1 + (TmpVal2 - 1)) / TmpVal2;
    }
    fpart->fat = part->start + fpart->reservedSectors;
    if (part->subtype == FS_FAT32)
    {
        fpart->rootFolder.handle.firstCluster = 2;
        fpart->root = fpart->fat + fpart->fatcopy*fpart->fatsize + fpart->SecPerCluster*(fpart->rootFolder.handle.firstCluster - 2);
        fpart->dataLBA = fpart->root;
    }
    else
    {
        fpart->rootFolder.handle.firstCluster = 0;
        fpart->root = fpart->fat + fpart->fatcopy*fpart->fatsize;
        fpart->dataLBA = fpart->root + fpart->maxroot / (part->disk->sectorSize / sizeof(FAT_dirEntry_t));
    }
    fpart->maxcls = part->size / fpart->SecPerCluster;

    // Write new file system
    uint8_t sector[512];

    part->disk->accessRemaining += 1;

    // Bootsector (sector 0)
    FAT_fillBootsector(part, sector);
    sectorWrite(part->start + 0, sector, part->disk);

    uint32_t FAT1sec = fpart->reservedSectors;
    uint32_t FAT2sec = FAT1sec + fpart->fatsize;
    if (part->subtype == FS_FAT12)
    {
        part->disk->accessRemaining += 35;

        // Sectors 1 - 18 (FAT1 and FAT2)
        memset(sector, 0, 512);
        for (uint8_t i = 1; i < 19; i++)
        {
            if (i == FAT1sec || i == FAT2sec)
            {
                if (part->disk->type == &FLOPPYDISK || part->disk->type == &USB_MSD)
                    sector[0] = 0xF0;
                else
                    sector[0] = 0xF8;
                sector[1] = 0xFF;
                sector[2] = 0xFF;
            }
            else
            {
                sector[0] = 0;
                sector[1] = 0;
                sector[2] = 0;
            }
            sectorWrite(part->start + i, sector, part->disk);
        }

        /// Prepare root directory (sector 19)
        strncpyandfill((char*)sector, part->name, 11, ' '); // Volume label
        sector[11] = ATTR_VOLUME | ATTR_ARCHIVE; // Attribute
        sectorWrite(part->start + 19, sector, part->disk);

        // Clear sectors 20 - 32
        memset(sector, 0, 12);
        for (uint8_t i = 20; i < 33; i++)
            sectorWrite(part->start + i, sector, part->disk);

        // Set sectors 33 - 35 to format ID of MS Windows
        memset(sector, 0xF6, 512); // format ID of MS Windows
        for (uint8_t i = 33; i < 36; i++)
            sectorWrite(part->start + i, sector, part->disk);
    }
    else if (part->subtype == FS_FAT16)
    {
        part->disk->accessRemaining += fpart->fatsize * 2 + 1;

        // Sectors FAT1sec - (FAT2sec + fpart->fatsize - 1) // FAT1 and FAT2
        memset(sector, 0, 512);
        for (uint32_t i = FAT1sec; i < FAT2sec + fpart->fatsize; i++)
        {
            if (i == FAT1sec || i == FAT2sec)
            {
                if (part->disk->type == &FLOPPYDISK || part->disk->type == &USB_MSD)
                    sector[0] = 0xF0;
                else
                    sector[0] = 0xF8;
                sector[1] = 0xFF;
                sector[2] = 0xFF;
                sector[3] = 0xFF;
                sectorWrite(part->start + i, sector, part->disk);
                sector[0] = 0;
                sector[1] = 0;
                sector[2] = 0;
                sector[3] = 0;
            }
            else
            {
                sectorWrite(part->start + i, sector, part->disk);
            }
        }

        /// Prepare root directory
        strncpyandfill((char*)sector, part->name, 11, ' '); // Volume label
        sector[11] = ATTR_VOLUME | ATTR_ARCHIVE; // Attribute
        sectorWrite(fpart->root, sector, part->disk);
    }
    else if (part->subtype == FS_FAT32)
    {
        part->disk->accessRemaining += 5 + fpart->fatsize*2 + 1;

        sectorWrite(part->start + 6, sector, part->disk); // backup of bootsector

        memset(sector, 0, 512);
        sector[510] = 0x55; // Boot signature
        sector[511] = 0xAA; // ...
        sectorWrite(part->start + 1, sector, part->disk); // TODO: later FSInfo has to be added
        sectorWrite(part->start + 2, sector, part->disk);
        sectorWrite(part->start + 7, sector, part->disk);
        sectorWrite(part->start + 8, sector, part->disk);

        // Sectors FAT1sec - (FAT2sec + fpart->fatsize - 1) // FAT1 and FAT2
        memset(sector, 0, 512);
        for (uint32_t i = FAT1sec; i < FAT2sec + fpart->fatsize; i++)
        {
            if (i == FAT1sec || i == FAT2sec)
            {
                // cf.
                if (part->disk->type == &FLOPPYDISK || part->disk->type == &USB_MSD)
                    sector[0]  = 0xF0;
                else
                    sector[0] = 0xF8;
                sector[1]  = 0xFF;
                sector[2]  = 0xFF;
                sector[3]  = 0x0F; // Media descriptor

                sector[4]  = 0xFF;
                sector[5]  = 0xFF;
                sector[6]  = 0xFF;
                sector[7]  = 0xFF;

                sector[8]  = 0xFF;
                sector[9]  = 0xFF;
                sector[10] = 0xFF;
                sector[11] = 0x0F; // Media descriptor

                sectorWrite(part->start + i, sector, part->disk);

                memset(sector, 0, 512); // reset buffer
            }
            else
            {
                sectorWrite(part->start + i, sector, part->disk);
            }
        }

        /// Prepare root directory
        strncpyandfill((char*)sector, part->name, 11, ' '); // Volume label
        sector[11] = ATTR_VOLUME | ATTR_ARCHIVE; // Attribute
        sectorWrite(fpart->root, sector, part->disk);
    }

    // DONE
    printf("Quickformat complete.\n");
    serial_log(SER_LOG_FAT, "Quickformat complete.\n");
    return (CE_GOOD);
}

FS_ERROR FAT_pinstall(partition_t* part)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_pinstall <<<<<");
  #endif

    FAT_partition_t* fpart = malloc(sizeof(FAT_partition_t), 0, "FAT_partition_t");
    part->data             = fpart;
    fpart->part            = part;

    uint8_t buffer[512];
    FS_ERROR err = singleSectorRead(part->start, buffer, part->disk);
    if (err != CE_GOOD)
    {
        free(fpart);
        return err;
    }

    BPBbase_t* BPB     = (BPBbase_t*)buffer;
    BPB1216_t* BPB1216 = (BPB1216_t*)buffer;
    BPB32_t* BPB32     = (BPB32_t*)  buffer;

    // Determine subtype (HACK: unrecommended way to determine type. cf. FAT specification)
    if (BPB1216->FStype[0] == 'F' && BPB1216->FStype[1] == 'A' && BPB1216->FStype[2] == 'T' && BPB1216->FStype[3] == '1' && BPB1216->FStype[4] == '2')
    {
        printf("FAT12");
        part->subtype = FS_FAT12;
        fpart->type   = FAT12;
    }
    else if (BPB1216->FStype[0] == 'F' && BPB1216->FStype[1] == 'A' && BPB1216->FStype[2] == 'T' && BPB1216->FStype[3] == '1' && BPB1216->FStype[4] == '6')
    {
        printf("FAT16");
        part->subtype = FS_FAT16;
        fpart->type   = FAT16;
    }
    else
    {
        printf("FAT32");
        part->subtype = FS_FAT32;
        fpart->type   = FAT32;
    }

    if (BPB->TotalSectors16 == 0)
        part->size = BPB->TotalSectors32;
    else
        part->size = BPB->TotalSectors16;

    fpart->fatcopy         = BPB->FATcount;
    fpart->SecPerCluster   = BPB->SectorsPerCluster;
    fpart->maxroot         = BPB->MaxRootEntries;
    fpart->reservedSectors = BPB->ReservedSectors;
    fpart->fat             = part->start + BPB->ReservedSectors;
    part->name             = malloc(12, 0, "part->name");
    part->name[11]         = 0;

    fpart->rootFolder.folder = part->rootFolder;
    part->rootFolder->data   = &fpart->rootFolder;

    if (part->subtype == FS_FAT32)
    {
        fpart->rootFolder.handle.firstCluster = BPB32->rootCluster;
        fpart->fatsize           = BPB32->FATsize32;
        fpart->root              = fpart->fat + fpart->fatcopy*fpart->fatsize + fpart->SecPerCluster*(fpart->rootFolder.handle.firstCluster-2);
        fpart->dataLBA           = fpart->root;
        part->serial             = BPB32->VolID;
        memcpy(part->name, &BPB32->VolLabel, 11);

      #ifdef _FAT_DIAGNOSIS_
        printf("\nFAT32 result: root: %u dataLBA: %u start: %u", fpart->root, fpart->dataLBA, fpart->part->start);
      #endif
    }
    else
    {
        fpart->rootFolder.handle.firstCluster = 0;
        fpart->fatsize           = BPB->FATsize16;
        fpart->root              = fpart->fat + fpart->fatcopy*fpart->fatsize;
        fpart->dataLBA           = fpart->root + fpart->maxroot/(part->disk->sectorSize/sizeof(FAT_dirEntry_t));
        part->serial             = BPB1216->VolID;
        memcpy(part->name, &BPB1216->VolLabel, 11);

      #ifdef _FAT_DIAGNOSIS_
        printf("\nFAT12/16 result: root: %u dataLBA: %u start: %u", fpart->root, fpart->dataLBA, fpart->part->start);
      #endif
    }

    fpart->maxcls = part->size/fpart->SecPerCluster;
    return (CE_GOOD);
}

FS_ERROR FAT_pfree(partition_t* part)
{
  #ifdef _FAT_DIAGNOSIS_
    serial_log(SER_LOG_FAT, "\n>>>>> FAT_pfree <<<<<");
  #endif

    free(part->name);
    free(part->data);

    return (CE_GOOD);
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
