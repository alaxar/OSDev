#ifndef TASK_H
#define TASK_H

#include "video/console.h"
#include "paging.h"
#include "scheduler.h"
#include "events.h"


typedef enum
{
    PROCESS, THREAD, VM86
} taskType_t;

struct task
{
    taskType_t       type;           // Indicates whether it is a thread or a process
    uint32_t         pid;            // Process ID
    uint32_t         esp;            // Stack pointer
    uint32_t         ss;             // Stack segment
    pageDirectory_t* pageDirectory;  // Page directory
    void*            kernelStack;    // Kernel stack location
    void*            FPUptr;         // Pointer to FPU data
    list_t*          threads;        // All threads owned by this tasks - deleted if this task is exited
    task_t*          parent;         // Task that created this thread (only used for threads)
    event_queue_t*   eventQueue;     // 0 if no event handling enabled. Points to queue otherwise.
    uint8_t          privilege;      // Access privilege
    struct ipc_node* ipc_node;       // IPC node of the current task
    struct ipc_node* ipc_working;    // Working IPC node of the current task

    // Information needed by scheduler
    //uint16_t         priority; // Indicates how often this task gets the CPU
    blocker_t        blocker;  // Object indicating reason and duration of blockade

    // Task specific graphical output settings
    console_t*       console; // Console used by this task
    uint8_t          attrib;  // Color

    // Information needed for cleanup
    bool    speaker;
    list_t* files;
    list_t* folders;
    list_t* heaps;

    // comment for debug purpose
    char comment[21];
};


extern task_t   kernelTask;
extern bool     task_switching;
extern task_t*  currentTask;


void     tasking_install(void);
task_t*  create_task(taskType_t type, pageDirectory_t* directory, void(*entry)(void), uint8_t privilege, console_t* console, size_t argc, char* argv[], const char* comment); // Creates a basic task
task_t*  create_thread (void(*entry)(void), const char* comment);                                                                                // Creates thread using currentTask's console
task_t*  create_cthread(void(*entry)(void), const char* consoleName);                                                                            // Creates a thread with a new console
task_t*  create_process (pageDirectory_t* directory, void(*entry)(void), uint8_t privilege, size_t argc, char* argv[], const char* comment);     // Creates a process which uses currentTask's console
task_t*  create_cprocess(pageDirectory_t* directory, void(*entry)(void), uint8_t privilege, size_t argc, char* argv[], const char* consoleName); // Creates a process with a new console
task_t*  create_vm86_task(pageDirectory_t* pd, void(*entry)(void), const char* comment);                                                         // Creates a task running in VM86 mode
void     task_passStackParameter(task_t* task, void* data, size_t length);                                                                       // Inserts a parameter to task's kernel stack
void     switch_context(bool force);
void     task_saveState(uint32_t esp);
uint32_t task_switch(task_t* newTask);
void     exit(void);
void     kill(task_t* task);
void     task_kill(uint32_t pid);
bool     waitForTask(const task_t* blockingTask, uint32_t timeout); // Returns false in case of timeout
uint32_t getpid(void);
void     task_log(const task_t* t);


#endif
