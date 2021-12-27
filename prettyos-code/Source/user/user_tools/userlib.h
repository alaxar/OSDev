#ifndef USERLIB_H
#define USERLIB_H

#include "types.h"


#define ERROR          LIGHT_RED
#define SUCCESS        GREEN
#define HEADLINE       CYAN
#define TABLE_HEADING  LIGHT_GRAY
#define DATA           BROWN
#define IMPORTANT      YELLOW
#define TEXT           WHITE
#define FOOTNOTE       LIGHT_RED
#define TITLEBAR       LIGHT_RED


#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))

// hton = Host To Network
static inline uint16_t htons(uint16_t v)
{
    __asm__("xchg %h0, %b0" : "+abcd"(v));
    return (v);
}
static inline uint32_t htonl(uint32_t v)
{
    __asm__("bswap %0" : "+r"(v));
    return (v);
}
// ntoh = Network To Host
#define ntohs(v) htons(v)
#define ntohl(v) htonl(v)

enum COLORS
{
    BLACK, BLUE,        GREEN,       CYAN,       RED,       MAGENTA,       BROWN,  LIGHT_GRAY,
    GRAY,  LIGHT_BLUE,  LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED, LIGHT_MAGENTA, YELLOW, WHITE
};

struct file;

// syscalls (only non-standard functions, because we do not want to include stdio.h here.
FS_ERROR execute(const char* path, size_t argc, char* argv[], bool ownConsole);
void exitProcess(void);
bool wait(BLOCKERTYPE reason, void* data, uint32_t timeout);

void* userHeap_create(size_t initSize);
void userHeap_destroy(void* handle);
void* userHeap_alloc(void* handle, size_t size);
void userHeap_free(void* handle, void* address);

FS_ERROR partition_format(const char* path, FS_t type, const char* name);
int fmove(const char* oldpath, const char* newpath, bool copy);
folder_t* folderAccess(const char* path, folderAccess_t mode);
void folderClose(folder_t* folder);

bool waitForEvent(uint32_t timeout);
void event_enable(bool b);
EVENT_t event_poll(void* destination, size_t maxLength, EVENT_t filter);

uint32_t getCurrentMilliseconds(void);

void systemControl(SYSTEM_CONTROL todo);

void textColor(uint8_t color);
void setScrollField(uint8_t top, uint8_t bottom);
void setCursor(position_t pos);
void getCursor(position_t* pos);
void clearScreen(uint8_t backgroundColor);
void console_setProperties(console_properties_t properties);
void refreshScreen(void);

bool keyPressed(KEY_t key);
bool mouseButtonPressed(mouse_button_t button);
void getMousePosition(position_t* pos);
void setMousePosition(int32_t x, int32_t y);

void beep(uint32_t frequency, uint32_t duration);

uint32_t getMyIP(void);
void dns_setServer(IP4_t server);
void dns_getServer(IP4_t* server);

uint32_t tcp_connect(IP4_t IP, uint16_t port);
bool     tcp_send(uint32_t ID, const void* data, size_t length);
bool     tcp_close(uint32_t ID);

bool udp_bind(uint16_t port);
bool udp_unbind(uint16_t port);
bool udp_send(const void* data, uint32_t length, IP4_t destIP, uint16_t srcPort, uint16_t destPort);

IPC_ERROR ipc_fopen(const char* path, file_t** destination, const char* mode);
IPC_ERROR ipc_getFolder(const char* path, char* destination, size_t length);
IPC_ERROR ipc_getString(const char* path, char* destination, size_t* length);
IPC_ERROR ipc_setString(const char* path, const char* source);
IPC_ERROR ipc_getInt(const char* path, int64_t* destination);
IPC_ERROR ipc_setInt(const char* path, const int64_t* source, IPC_TYPE type);
IPC_ERROR ipc_getDouble(const char* path, double* destination);
IPC_ERROR ipc_setDouble(const char* path, const double* source);
IPC_ERROR ipc_deleteKey(const char* path);
IPC_ERROR ipc_setAccess(const char* path, IPC_RIGHTS permissions, uint32_t task);
IPC_ERROR ipc_setWorkingNode(const char* path, bool create);

// deprecated
int32_t floppy_dir(const char* path);
void printLine(const char* message, uint16_t line, uint8_t attribute);


// user functions
void event_flush(EVENT_t filter);
void sleep(uint32_t milliseconds);
bool waitForTask(uint32_t pid, uint32_t timeout);

void iSetCursor(uint16_t x, uint16_t y);
uint32_t getCurrentSeconds(void);

int strncicmp(const char* s1, const char* s2, size_t n);

void  reverse(char* s);
char* itoa(int32_t n, char* s);
char* utoa(uint32_t n, char* s);
void  ftoa(float f, char* buffer);
void  i2hex(uint32_t val, char* dest, uint32_t len);

void showInfo(uint8_t val);

IP4_t stringToIP(char* str);

bool squashPath(char* path);

bool changeWorkingDir(const char* subpath);
char* getWorkingDir(void);


#endif
