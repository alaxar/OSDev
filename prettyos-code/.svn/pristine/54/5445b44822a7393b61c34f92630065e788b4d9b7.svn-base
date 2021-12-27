/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "task.h"
#include "util/util.h"
#include "memory.h"
#include "descriptor_tables.h"
#include "kheap.h"
#include "netprotocol/udp.h"
#include "netprotocol/tcp.h"
#include "filesystem/fsmanager.h"
#include "audio/sys_speaker.h"
#include "userheap.h"
#include "cpu.h"
#include "ipc.h"


// Kernel stack size
static const uint32_t kernelStackSize = 0x1000; // Tasks get a 4 KB kernel stack


bool task_switching = false; // We allow task switching when tasking and scheduler are installed.

task_t kernelTask =
{
    .type          = PROCESS,
    .pid           = 0,
    .esp           = 0,
    .ss            = 0x10,
    .kernelStack   = 0,      // The kernel task does not need a kernel stack because it does not call his own functions by syscall
    .FPUptr        = 0,
    .threads       = 0,      // No threads associated with the task at the moment. List is created later if necessary
    .parent        = 0,
    .eventQueue    = 0,
    .privilege     = 0,
    .ipc_node      = 0,
    .ipc_working   = 0,

    .blocker.type  = 0,      // The task is not blocked (scheduler.h/c)

    .console       = &kernelConsole,
    .attrib        = 0x0F,

    .speaker       = false,
    .files         = 0,
    .folders       = 0,
    .heaps         = 0,

    .comment       = "kernel"
};

task_t* currentTask = &kernelTask;
list_t tasks = list_init(); // List of all tasks. Not sorted by pid

static uint32_t next_pid = 1; // The next available process ID (kernel has 0, so we start with 1 here).


void tasking_install(void)
{
  #ifdef _TASKING_DIAGNOSIS_
    textColor(TEXT);
    printf("Install tasking\n");
    textColor(TEXT);
  #endif

    kernelTask.pageDirectory = kernelPageDirectory;
    kernelTask.eventQueue = event_createQueue();

    list_append(&kernelTask.console->tasks, &kernelTask);
    list_append(&tasks, &kernelTask);

    scheduler_insertTask(&kernelTask);

    task_switching = true; // Finished installing scheduler and tasking - allow to switch tasks
}

uint32_t getpid(void)
{
    return (currentTask->pid);
}

bool waitForTask(const task_t* blockingTask, uint32_t timeout)
{
    if (timeout > 0)
    {
        return scheduler_blockCurrentTask(BL_TASK, (void*)blockingTask->pid, max(1, timeout));
    }

    scheduler_blockCurrentTask(BL_TASK, (void*)blockingTask->pid, 0);
    return (true);
}


/// Functions to create tasks
task_t* create_task(taskType_t type, pageDirectory_t* directory, void(*entry)(void), uint8_t privilege, console_t* console, size_t argc, char* argv[], const char* comment)
{
    task_t* newTask = malloc(sizeof(task_t), 0, "task_t");
    newTask->type          = type;
    newTask->pid           = next_pid++;
    newTask->pageDirectory = directory;
    newTask->privilege     = privilege;
    newTask->FPUptr        = 0;
    newTask->attrib        = 0x0F;
    newTask->blocker.type  = 0;
    newTask->threads       = 0; // No threads associated with the task at the moment. created later if necessary
    newTask->eventQueue    = 0; // Event handling is disabled per default
    newTask->files         = 0;
    newTask->folders       = 0;
    newTask->heaps         = list_create();
    newTask->speaker       = false;
    strncpy(newTask->comment, comment, 20);
    newTask->comment[20] = 0;

    // TODO: A ring 3 thread needs a different stack address. But in fact, it is pointless to assign a fixed stack address anyway.
    if (newTask->privilege == 3 && newTask->type != VM86)
        paging_alloc(newTask->pageDirectory, (void*)(USER_STACK - 10*PAGESIZE), 10*PAGESIZE, MEM_USER|MEM_WRITE); // Stack starts at USER_STACK-StackSize*PAGESIZE

    newTask->kernelStack = malloc(kernelStackSize, 4, "task-kernelstack") + kernelStackSize;
    uint32_t* kernelStack = newTask->kernelStack;

    uint32_t code_segment = 0x08;

    if (newTask->type != VM86)
    {
        if (newTask->privilege == 0)
        {
            *(--kernelStack) = (uintptr_t)&exit; // When a ring0-task is finished, exit is automatically called
        }
        else if (newTask->privilege == 3)
        {
            // General information: Intel 3A Chapter 5.12
            *(--kernelStack) = 0x23;       // ss
            *(--kernelStack) = USER_STACK; // esp
            code_segment = 0x1B;           // 0x18|0x3=0x1B
        }

        *(--kernelStack) = 0x0202; // eflags: interrupts activated, iopl = 0
    }
    else
    {
        code_segment = 0;
        *(--kernelStack) = 0x0000;  // ss
        *(--kernelStack) = 0x6000;  // esp
        *(--kernelStack) = 0x20202; // eflags = vm86 (bit17), interrupts (bit9), iopl=0
    }

    *(--kernelStack) = code_segment;    // cs
    *(--kernelStack) = (uint32_t)entry; // eip
    *(--kernelStack) = 0;               // error code
    *(--kernelStack) = 0;               // Interrupt nummer

    // General purpose registers w/o esp
    *(--kernelStack) = argc;            // eax. Used to give argc to user programs.
    *(--kernelStack) = (uintptr_t)argv; // ecx. Used to give argv to user programs.
    *(--kernelStack) = 0; //cpu_supports(CF_SYSENTEREXIT); // edx. Used to inform the user programm about the support for the SYSENTER/SYSEXIT instruction. TODO: Enable it. At the moment, there are problems with multiple tasks
    *(--kernelStack) = 0;
    *(--kernelStack) = 0;
    *(--kernelStack) = 0;
    *(--kernelStack) = 0;

    const uint32_t data_segment = newTask->privilege == 3 ? 0x23 : 0x10;

    *(--kernelStack) = data_segment;
    *(--kernelStack) = data_segment;
    *(--kernelStack) = data_segment;
    *(--kernelStack) = data_segment;

    newTask->esp = (uint32_t)kernelStack;
    newTask->ss  = data_segment;

    newTask->console = console;
    if (console)
        list_append(&console->tasks, newTask);

    list_append(&tasks, newTask);

  #ifdef _TASKING_DIAGNOSIS_
    task_log(newTask);
  #endif

    if (type == PROCESS)
    {
        // Create IPC node for task
        char path[30];
        snprintf(path, 30, "Tasks|%u", newTask->pid);
        ipc_createNode(path, &newTask->ipc_node, IPC_FOLDER);
        newTask->ipc_node->owner = newTask->pid;

        // Fill in basic data (pid, etc.)
        char* pathEnd = path + strlen(path);
        strcpy(pathEnd, "|pid");
        int64_t pid = newTask->pid;
        ipc_setInt(path, &pid, IPC_UINT);

        strcpy(pathEnd, "|workingDir");
        ipc_setString(path, "1:0:|");
        ipc_setAccess(path, IPC_READ | IPC_WRITE | IPC_DELEGATERIGHTS, newTask->pid);
    }
    else
        newTask->ipc_node = 0;
    newTask->ipc_working = 0;

    return (newTask);
}

task_t* create_cprocess(pageDirectory_t* directory, void(*entry)(void), uint8_t privilege, size_t argc, char* argv[], const char* consoleName)
{
  #ifdef _TASKING_DIAGNOSIS_
    printf("create ctask");
  #endif

    console_t* console = malloc(sizeof(console_t), 0, "task-console");
    console_init(console, consoleName);
    task_t* newTask = create_task(PROCESS, directory, entry, privilege, console, argc, argv, consoleName);

    newTask->eventQueue = event_createQueue(); // For tasks event handling is enabled per default

    return (newTask);
}

task_t* create_process(pageDirectory_t* directory, void(*entry)(void), uint8_t privilege, size_t argc, char* argv[], const char* comment)
{
  #ifdef _TASKING_DIAGNOSIS_
    printf("create task");
  #endif

    task_t* newTask = create_task(PROCESS, directory, entry, privilege, currentTask->console, argc, argv, comment); // task shares the console of the current task

    newTask->eventQueue = event_createQueue(); // For tasks event handling is enabled per default

    return (newTask);
}

task_t* create_cthread(void(*entry)(void), const char* consoleName)
{
  #ifdef _TASKING_DIAGNOSIS_
    printf("create cthread");
  #endif

    console_t* console = malloc(sizeof(console_t), 0, "thread-console");
    console_init(console, consoleName);
    task_t* newTask = create_task(THREAD, currentTask->pageDirectory, entry, currentTask->privilege, console, 0, 0, consoleName);

    // Attach the thread to its parent
    newTask->parent = currentTask;

    if (currentTask->threads == 0)
    {
        currentTask->threads = list_create();
    }

    list_append(currentTask->threads, newTask);

    newTask->eventQueue = event_createQueue(); // Every thread with an own console gets an own eventQueue, because otherwise lots of input will never arrive.

    return (newTask);
}

task_t* create_thread(void(*entry)(void), const char* comment)
{
  #ifdef _TASKING_DIAGNOSIS_
    printf("create thread");
  #endif

    task_t* newTask = create_task(THREAD, currentTask->pageDirectory, entry, currentTask->privilege, currentTask->console, 0, 0, comment); // task shares the console of the current task

    // Attach the thread to its parent
    newTask->parent = currentTask;

    if (currentTask->threads == 0)
    {
        currentTask->threads = list_create();
    }

    list_append(currentTask->threads, newTask);

    return (newTask);
}

task_t* create_vm86_task(pageDirectory_t* pd, void(*entry)(void), const char* comment)
{
  #ifdef _TASKING_DIAGNOSIS_
    printf("create vm86 task");
  #endif

    task_t* newTask = create_task(VM86, pd, entry, 3, currentTask->console, 0, 0, comment); // task shares the console of the current task

    return (newTask);
}

void task_passStackParameter(task_t* task, void* data, size_t length)
{
    // Function injects further parameters at stack end
    void* stack_top = (void*)task->esp;
    void* stack_end = task->kernelStack;
    void* stack_begin = task->kernelStack-kernelStackSize;
    size_t move_by = min(length, (size_t)(stack_top-stack_begin));

    memmove(stack_top-move_by, stack_top, (size_t)(stack_end-stack_top)); // Move old stack content
    memcpy(stack_end-move_by, data, move_by); // Inject parameter

    task->esp -= move_by; // Move stack pointer
}


/// Functions to switch the task
void task_saveState(uint32_t esp)
{
    currentTask->esp = esp;
}

uint32_t task_switch(task_t* newTask)
{
    task_switching = false;

    currentTask = newTask;

    tss_switch((uintptr_t)currentTask->kernelStack, currentTask->esp, currentTask->ss); // esp0, esp, ss

    paging_switch(currentTask->pageDirectory);

    #ifdef _TASKING_DIAGNOSIS_
    textColor(TEXT);
    printf("%u ", currentTask->pid);
    textColor(TEXT);
    #endif

    // Set TS
    if (currentTask == FPUTask)
    {
        __asm__ volatile("clts"); // CLearTS: reset the TS bit (no. 3) in CR0 to disable #NM
    }
    else
    {
        uint32_t cr0;
        __asm__("mov %%cr0, %0": "=r"(cr0)); // Read cr0
        cr0 |= BIT(3); // Set the TS bit (no. 3) in CR0 to enable #NM (exception no. 7)
        __asm__("mov %0, %%cr0":: "r"(cr0)); // Write cr0
    }

    task_switching = true;

    return (currentTask->esp); // Return new task's esp
}

void switch_context(bool force) // Switch to next task (by interrupt)
{
    if (force || scheduler_shouldSwitchTask()) // If the scheduler wants to switch the task, or if a task switch is enforced ...
        __asm__ volatile("int $0x7E"); // Call interrupt that will call task_switch.
    else // Otherwise just idle and continue
    {
        sti();
        hlt(); // Wait one cycle
    }
}


// Functions to kill a task
void kill(task_t* task)
{
    bool temp_task_switching = task_switching;
    task_switching = false; // There should not occur a task switch while we are exiting from a task, to avoid data corruption

  #ifdef _TASKING_DIAGNOSIS_
    scheduler_log();
  #endif

    // Cleanup
    console_cleanup(task);
    udp_cleanup(task);
    tcp_cleanup(task);
    fsmanager_cleanup(task);

    if (task->speaker)
        noSound();

    userHeap_cleanUp(task);

    // Free user memory, if this task has an own PD
    if (task->type != THREAD && task->type != VM86 && task->pageDirectory != kernelPageDirectory)
    {
        paging_destroyPageDirectory(task->pageDirectory);
    }

    // Inform the parent task that this task has exited
    if (task->type == THREAD)
    {
        list_delete(task->parent->threads, list_find(task->parent->threads, task));
    }

    // Kill all child-threads of this task
    if (task->threads)
    {
        for (dlelement_t* e = task->threads->head; e != 0; e = e->next)
        {
            kill(e->data);
        }
        list_free(task->threads);
    }

    list_delete(&tasks, list_find(&tasks, task));
    scheduler_deleteTask(task);

  #ifdef _TASKING_DIAGNOSIS_
    textColor(TEXT);
    printf("exit finished.\n");
    scheduler_log();
  #endif

    if (task->eventQueue != 0)
    {
        event_deleteQueue(task->eventQueue);
    }

    if (task == &kernelTask)
    {
        systemControl(REBOOT);
    }

    if (task->ipc_node)
        ipc_deleteNode(task->ipc_node);

    free(task->FPUptr);
    free(task->kernelStack - kernelStackSize); // Free kernelstack
    free(task);

    task_switching = temp_task_switching;

    if (task == currentTask) // Tasks adress is still saved, although its no longer valid so we can use it here
    {
        switch_context(true); // Switch to next task
    }
}

void task_kill(uint32_t pid)
{
    // Find out task by looking for the pid in the tasks-list
    for (dlelement_t* e = tasks.head; e != 0; e = e->next)
    {
        if (((task_t*)e->data)->pid == pid)
        {
            kill(e->data);
            return;
        }
    }
}

void exit(void)
{
    kill(currentTask);
}

void task_log(const task_t* t)
{
    textColor(IMPORTANT);
    printf("%u\t\b\b\b\b\b", t->pid);     // Process ID (pid)
    textColor(TEXT);
    printf("%X ", t->esp);           // Stack pointer
    printf("%X ", t->pageDirectory); // Page directory

    if (t->kernelStack)
    {
        printf("%X ", t->kernelStack); // Kernel stack location
    }
    else
    {
        textColor(GRAY);
        printf("-------- ");
        textColor(TEXT);
    }

    if (t->privilege)
    {
        printf("user   ");
    }
    else
    {
        printf("kernel ");
    }

    textColor(DATA);
    puts(t->comment);
    textColor(TEXT);
    position_t pos;
    getCursor(&pos);
    pos.x = 58;
    setCursor(pos);

    if (t->type == THREAD)
    {
        printf("parent: ");

        textColor(IMPORTANT);
        printf("%u ", t->parent->pid);
        textColor(TEXT);
    }

    if (t->threads && t->threads->head)
    {
        printf("children:");

        textColor(IMPORTANT);
        for (dlelement_t* e = t->threads->head; e != 0; e = e->next)
        {
            printf(" %u", ((task_t*)e->data)->pid);
        }
        textColor(TEXT);
    }

    putch('\n');
}


/*
* Copyright (c) 2009-2017 The PrettyOS Project. All rights reserved.
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
