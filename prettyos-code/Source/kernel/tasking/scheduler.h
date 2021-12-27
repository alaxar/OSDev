#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "os.h"


typedef struct task task_t;

typedef struct
{
    bool (*unlock)(void*); // if 0, the blocker is event based
} blockerType_t;

typedef enum
{
    BL_TIME, BL_SYNC, BL_INTERRUPT, BL_TASK, BL_TODOLIST, BL_EVENT, BL_NETPACKET
} BLOCKERTYPE;

typedef struct
{
    const blockerType_t* type;
    void*          data;    // While the task is blocked, it contains information for the unblock functions. After the block ended, it is set to 0 in case of timeout and 1 otherwise
    uint32_t       timeout; // 0 if no timeout is defined.
} blocker_t;


bool     scheduler_shouldSwitchTask(void);
uint32_t scheduler_taskSwitch(uint32_t esp);
void     scheduler_insertTask(task_t* task);
void     scheduler_deleteTask(task_t* task);
bool     scheduler_blockCurrentTask(BLOCKERTYPE, void* data, uint32_t timeout); // false in case of timeout
void     scheduler_unblockEvent(BLOCKERTYPE type, void* data);
void     scheduler_log(void);


#endif
