/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "synchronisation.h"
#include "kheap.h"
#include "task.h"


// Semaphore
semaphore_t* semaphore_create(uint16_t resourceCount)
{
    semaphore_t* obj = malloc(sizeof(semaphore_t), 0, "semaphore");
    obj->resCount = resourceCount;
    obj->freeRes = obj->resCount;
    return (obj);
}

void semaphore_construct(semaphore_t* obj, uint16_t resourceCount)
{
    obj->resCount = resourceCount;
    obj->freeRes = obj->resCount;
}

void semaphore_lock(semaphore_t* obj)
{
    if (obj == 0) return; // Invalid object

    while (obj->freeRes == 0) // blocked? -> wait. Do this in a loop to prevent two tasks locking a semaphore at the "same" time
        scheduler_blockCurrentTask(BL_SYNC, obj, 0);

    obj->freeRes--; // aquire one resource
}

void semaphore_unlock(semaphore_t* obj)
{
    if (obj == 0) return; // Invalid object

    if (obj->resCount == 0 || obj->freeRes < obj->resCount) // Protected against increasing the number of resources by unlocking it multiple times
        obj->freeRes++; // free one resource

    scheduler_unblockEvent(BL_SYNC, obj); // Inform scheduler that this semaphore has been unlocked
}

void semaphore_destruct(semaphore_t* obj)
{
    if (obj->freeRes == 0) // There can be tasks that are blocked due to this semaphore. Unlock them to avoid deadlocks
        scheduler_unblockEvent(BL_SYNC, obj);
    obj->freeRes = 0;
    obj->resCount = 0;
}

void semaphore_free(semaphore_t* obj)
{
    if (obj->freeRes == 0) // There can be tasks that are blocked due to this semaphore. Unlock them to avoid deadlocks
        scheduler_unblockEvent(BL_SYNC, obj);

    free(obj);
}


// Mutex
mutex_t* mutex_create(void)
{
    mutex_t* obj = malloc(sizeof(mutex_t), 0, "mutex");
    obj->blocks = 0;
    obj->blocker = 0;
    return (obj);
}

void mutex_construct(mutex_t* obj)
{
    obj->blocks = 0;
    obj->blocker = 0;
}

void mutex_lock(mutex_t* obj)
{
    bool temp_task_switching = task_switching;
    task_switching = false;
    if (obj->blocks != 0 && obj->blocker == currentTask) // Mutex has already been locked by this task. Increase blocks counter.
    {
        obj->blocks++;
        task_switching = temp_task_switching;
        return;
    }

    while (obj->blocks != 0)
    {
        task_switching = temp_task_switching;
        scheduler_blockCurrentTask(BL_SYNC, obj, 0); // Wait until the mutex is unlocked
        task_switching = false;
    }

    obj->blocks = 1;
    obj->blocker = currentTask;
    task_switching = temp_task_switching;
}

void mutex_unlock(mutex_t* obj)
{
    bool temp_task_switching = task_switching;
    task_switching = false;

    ASSERT(obj->blocks > 0);
    obj->blocks--; // Release one lock.

    task_switching = temp_task_switching;

    scheduler_unblockEvent(BL_SYNC, obj); // Inform scheduler that this mutex has been unlocked
}

void mutex_destruct(mutex_t* obj)
{
    if (obj->blocks != 0) // There can be tasks that are blocked due to this mutex. Unlock them to avoid deadlocks
        scheduler_unblockEvent(BL_SYNC, obj);
}

void mutex_free(mutex_t* obj)
{
    mutex_destruct(obj);
    free(obj);
}


/*
* Copyright (c) 2010-2016 The PrettyOS Project. All rights reserved.
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
