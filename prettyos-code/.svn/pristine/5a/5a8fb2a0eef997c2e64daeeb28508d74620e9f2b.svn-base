#ifndef FSMANAGER_H
#define FSMANAGER_H

#include "os.h"
#include "util/util.h"
#include "util/list.h"
#include "tasking/scheduler.h"


typedef enum
{
    FS_FAT12=1, FS_FAT16, FS_FAT32,
    FS_RAMFS
} FS_t;

typedef enum
{
    SEEK_SET, SEEK_CUR, SEEK_END
} SEEK_ORIGIN;

typedef enum
{
    FOLDER_OPEN, FOLDER_CREATE, FOLDER_DELETE
} folderAccess_t;

typedef enum
{
    CE_GOOD = 0,                    // No error
    CE_ERASE_FAIL,                  // An erase failed
    CE_NOT_PRESENT,                 // No device was present
    CE_NOT_FORMATTED,               // The disk is of an unsupported format
    CE_BAD_PARTITION,               // The boot record is bad

    CE_UNSUPPORTED_FS,              // The file system type is unsupported
    CE_BAD_FORMAT_PARAM,            // Trying to format a disk with bad parameters
    CE_NOT_INIT,                    // An operation was performed on an uninitialized device
    CE_BAD_SECTOR_READ,             // A bad read of a sector occured
    CE_WRITE_ERROR,                 // Could not write to a sector

    CE_INVALID_CLUSTER,             // Invalid cluster value > maxcls
    CE_FILE_NOT_FOUND,              // Could not find the file on the device
    CE_DIR_NOT_FOUND,               // Could not find the directory
    CE_BAD_FILE,                    // File is corrupted
    CE_TIMEOUT,                     // Timout while trying to access

    CE_COULD_NOT_GET_CLUSTER,       // Could not load/allocate next cluster in file
    CE_FILENAME_2_LONG,             // A specified file name is too long to use
    CE_FILENAME_EXISTS,             // A specified filename already exists on the device
    CE_INVALID_FILENAME,            // Invalid file name
    CE_DELETE_DIR,                  // The user tried to delete a directory with FSremove

    CE_DIR_FULL,                    // All root dir entry are taken
    CE_DISK_FULL,                   // All clusters in partition are taken
    CE_DIR_NOT_EMPTY,               // This directory is not empty yet, remove files before deleting
    CE_NONSUPPORTED_SIZE,           // The disk is too big to format as FAT16
    CE_WRITE_PROTECTED,             // Card is write protected

    CE_FILENOTOPENED,               // File not opened for the write
    CE_SEEK_ERROR,                  // File location could not be changed successfully
    CE_BADCACHEREAD,                // Bad cache read
    CE_UNSUPPORTED_FUNCTION,        // Driver does not support this operation
    CE_READONLY,                    // The file is read-only

    CE_WRITEONLY,                   // The file is write-only
    CE_INVALID_ARGUMENT,            // Invalid argument
    CE_FOPEN_ON_DIR,                // Attempted to call fopen() on a directory
    CE_UNSUPPORTED_SECTOR_SIZE,     // Unsupported sector size

    CE_FAT_EOF = 60,                // Read try beyond FAT's EOF
    CE_EOF                          // EOF
} FS_ERROR;

struct partition;
struct file;
struct folder;
typedef struct
{
    // Access-functions
    FS_ERROR (*pinstall)(struct partition*); // Partition
    FS_ERROR (*pfree)   (struct partition*); // Partition
    FS_ERROR (*pformat) (struct partition*); // Partition

    FS_ERROR (*fopen) (struct file*, bool, bool);                  // File, create if not existant, overwrite file before opening
    FS_ERROR (*fclose)(struct file*);                              // File
    FS_ERROR (*fseek) (struct file*, int32_t, SEEK_ORIGIN);        // File, offset, origin
    FS_ERROR (*fread) (struct file*, void*, size_t);               // File, destination, size
    FS_ERROR (*fwrite)(struct file*, const void*, size_t);         // File, source, size
    FS_ERROR (*fflush)(struct file*);                              // File
    FS_ERROR (*remove)(struct file*);                              // File
    FS_ERROR (*rename)(struct file*, struct folder*, const char*); // Old file, new folder, new name

    FS_ERROR (*folderAccess)(struct folder*, folderAccess_t); // Folder, mode
    void     (*folderClose) (struct folder*);                 // Folder

    // Structure-Information (used to create a partition of that format)
    uint8_t typeID;
} fileSystem_t;

struct disk;
typedef struct partition
{
    const fileSystem_t* type;  // Type of the partition. 0 = unformated
    uint32_t       subtype;    // Example: To detect wether its a FAT12, FAT16 or FAT32 device although it uses the same driver
    void*          data;       // data specific to partition-type
    struct folder* rootFolder; // Root of the file tree

    struct disk*  disk;    // The disk containing the partition
    uint32_t      serial;  // serial for identification
    char*         name;    // volume label

    uint32_t      start;   // First sector
    uint32_t      size;    // Total size of partition (in sectors)
    bool          mount;   // false: not mounted
} partition_t;

enum FSNODE_ATTRIBUTES
{
    NODE_VOLUME    = BIT(0),
    NODE_DIRECTORY = BIT(1),
    NODE_HIDDEN    = BIT(2),
    NODE_READONLY  = BIT(3),
    NODE_LINK      = BIT(4),
    NODE_SYSTEM    = BIT(5),
    NODE_ARCHIVE   = BIT(6)
};

typedef struct
{
    char*          name;       // Name of the node
    uint64_t       size;       // Total size of the file in bytes (0 for folders)
    struct folder* folderData; // Points to associated folder_t, if opened
    uint8_t        attributes; // Attributes
} fsnode_t;

typedef struct folder
{
    partition_t*   volume;    // Partition containing the folder
    struct folder* folder;    // Parent folder containing the folder
    void*          data;      // Additional information specific to fileSystem, e.g. FAT_folder

    char*          name;      // name of the node

    list_t         nodes;     // All files and folders inside this folder. Stored as fsnode_t
} folder_t;

typedef struct file
{
    partition_t* volume; // volume containing the file
    folder_t*    folder; // Folder containing the file (parent-folder)
    void*        data;   // Additional information specific to fileSystem

    uint32_t     seek;   // current byte in the file
    uint32_t     size;   // file size
    char*        name;   // name of the node

    FS_ERROR     error;  // Error-value, 0 if no error
    bool         write;  // file is opened for writing
    bool         read;   // file is opened for reading
    bool         EOF;    // process has reached end of file
    bool         binary; // Read file in binary mode
} file_t;


extern const fileSystem_t FAT, RAMFS;


// Partition functions
FS_ERROR formatPartition(const char* path, FS_t type, const char* name);
FS_ERROR analyzePartition(partition_t* part);
void     freePartition(partition_t* part);

// File functions
file_t* fopen (const char* path, const char* mode);
void    fclose_task(file_t* file, task_t* task);
void    fclose(file_t* file);

FS_ERROR fmove(const char* oldpath, const char* newpath, bool copy);

FS_ERROR fputc (char c,                                     file_t* file);
char     fgetc (                                            file_t* file);
char*    fgets (char* dest,      size_t num,                file_t* file);
FS_ERROR fputs (const char* src,                            file_t* file);
size_t   fread (void* dest,      size_t size, size_t count, file_t* file);
size_t   fwrite(const void* src, size_t size, size_t count, file_t* file);

FS_ERROR fflush(file_t* file);

size_t   ftell (file_t* file);
FS_ERROR fseek (file_t* file, int32_t offset, SEEK_ORIGIN origin);
FS_ERROR rewind(file_t* file);

bool feof(file_t* file);

FS_ERROR ferror  (file_t* file);
void     clearerr(file_t* file);

// Folder functions
folder_t* folderAccess(const char* path, folderAccess_t mode);
void      folderClose (folder_t* folder);

// General functions
void fsmanager_cleanup(task_t* task);
void fsmanager_log(void);

// Path functions
const char* getFilename(const char* path);

#endif
