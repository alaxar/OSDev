#ifndef SYNCHRONISATION_H
#define SYNCHRONISATION_H

#include "scheduler.h"


typedef struct
{
    uint16_t resCount;  // Number of resources
    uint16_t freeRes;   // Index of the first unused resource in the array mentioned above
} semaphore_t;

#define  semaphore_init(resourceCount) {resourceCount, 0}
semaphore_t* semaphore_create(uint16_t resourceCount);
void         semaphore_construct(semaphore_t* obj, uint16_t resourceCount);
void         semaphore_lock  (semaphore_t* obj);
void         semaphore_unlock(semaphore_t* obj);
void         semaphore_destruct(semaphore_t* obj);
void         semaphore_free(semaphore_t* obj);


typedef struct
{
    task_t*  blocker; // Task that is blocking the mutex
    uint32_t blocks;  // Indicates whether this mutex is blocked at the moment or not. -> You have to call unlock as often as lock to unblock mutex.
} mutex_t;

#define  mutex_init() {0, 0}
mutex_t* mutex_create(void);
void     mutex_construct(mutex_t* obj);
void     mutex_lock(mutex_t* obj);
void     mutex_unlock(mutex_t* obj);
void     mutex_destruct(mutex_t* obj);
void     mutex_free(mutex_t* obj);


#endif
