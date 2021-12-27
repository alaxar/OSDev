#ifndef TYPES_H
#define TYPES_H

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdarg.h"


typedef enum
{
    __KEY_INVALID,
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_ACC, KEY_MINUS, KEY_EQUAL, KEY_BACKSL, KEY_BACK, KEY_SPACE, KEY_CAPS, KEY_TAB, KEY_OSQBRA, KEY_CSQBRA,
    KEY_LSHIFT, KEY_LCTRL, KEY_LGUI, KEY_LALT, KEY_RSHIFT, KEY_RCTRL, KEY_RGUI, KEY_ALTGR, KEY_MENU, KEY_ENTER, KEY_ESC,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_PRINT, KEY_SCROLL, KEY_PAUSE, KEY_NUM, KEY_INS, KEY_DEL,
    KEY_HOME, KEY_PGUP, KEY_END, KEY_PGDWN, KEY_ARRU, KEY_ARRL, KEY_ARRD, KEY_ARRR,
    KEY_KPSLASH, KEY_KPMULT, KEY_KPMIN, KEY_KPPLUS, KEY_KPEN, KEY_KPDOT,
    KEY_KP0, KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP4, KEY_KP5, KEY_KP6, KEY_KP7, KEY_KP8, KEY_KP9,
    KEY_SEMI, KEY_APPOS, KEY_COMMA, KEY_DOT, KEY_SLASH,
    KEY_GER_ABRA, // German keyboard has one key more than the international one. TODO: Find a better name.
    __KEY_LAST
} KEY_t;

typedef enum
{
    BUTTON_LEFT = 1,
    BUTTON_MIDDLE = 2,
    BUTTON_RIGHT = 4,
    BUTTON_4 = 8,
    BUTTON_5 = 16
} mouse_button_t;

typedef enum
{
    EVENT_NONE, EVENT_BUFFER_TO_SMALL, EVENT_OVERFLOW,
    EVENT_KEY_DOWN, EVENT_KEY_UP, EVENT_TEXT_ENTERED,
    EVENT_MOUSE_BUTTON_DOWN, EVENT_MOUSE_BUTTON_UP,
    EVENT_MOUSE_MOVE, EVENT_MOUSE_WHEEL_MOVED,
    EVENT_TCP_CONNECTED, EVENT_TCP_RECEIVED, EVENT_TCP_CLOSED,
    EVENT_UDP_RECEIVED,
    EVENT_CONSOLE_GAINED_FOCUS, EVENT_CONSOLE_LOST_FOCUS
} EVENT_t;

typedef enum
{
    CONSOLE_FULLSCREEN  = 1,
    CONSOLE_SHOWINFOBAR = 2,
    CONSOLE_AUTOREFRESH = 4,
    CONSOLE_AUTOSCROLL  = 8
} console_properties_t;

typedef enum
{
    IPC_UINT, IPC_INT, IPC_QWORD, IPC_SIZE, IPC_FLOAT, IPC_STRING, IPC_FILE, IPC_FOLDER
} IPC_TYPE;

typedef enum
{
    IPC_SUCCESSFUL = 0,
    IPC_NOTENOUGHMEMORY, IPC_NOTFOUND, IPC_ACCESSDENIED, IPC_WRONGTYPE
} IPC_ERROR;

typedef enum
{
    IPC_NONE = 0,
    IPC_READ = 1, IPC_WRITE = 2, IPC_DELEGATERIGHTS = 4
} IPC_RIGHTS;

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

typedef enum
{
    FS_FAT12=1, FS_FAT16, FS_FAT32,
    FS_RAMFS
} FS_t;

typedef enum
{
    FOLDER_OPEN, FOLDER_CREATE, FOLDER_DELETE
} folderAccess_t;

typedef struct folder folder_t;

typedef enum
{
    BL_TIME, BL_SYNC, BL_INTERRUPT, BL_TASK, BL_TODOLIST, BL_EVENT, BL_NETPACKET
} BLOCKERTYPE;

typedef enum
{
    STANDBY, SHUTDOWN, REBOOT
} SYSTEM_CONTROL;

typedef struct
{
    uint16_t x, y;
} position_t;

typedef union
{
    uint8_t IP4[4];
    uint32_t iIP4;
} __attribute__((packed)) IP4_t;

typedef struct
{
    uint32_t connectionID;
    IP4_t sourceIP;
    uint16_t sourcePort;
} __attribute__((packed)) tcpConnectedEventHeader_t;

typedef struct
{
    uint32_t connectionID;
    size_t   length;
} __attribute__((packed)) tcpReceivedEventHeader_t;

typedef struct
{
    IP4_t     srcIP;
    uint16_t srcPort;
    uint16_t destPort;
    size_t   length;
} __attribute__((packed)) udpReceivedEventHeader_t;


struct file;
typedef struct file file_t;


#endif
