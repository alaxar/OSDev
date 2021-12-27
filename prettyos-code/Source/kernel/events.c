/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "events.h"
#include "util/util.h"
#include "kheap.h"
#include "tasking/task.h"


event_queue_t* event_createQueue(void)
{
    event_queue_t* queue = malloc(sizeof(event_queue_t), 0, "event_queue");
    queue->num = 0;
    mutex_construct(&queue->mutex);
    list_construct(&queue->list);
    return (queue);
}

void event_deleteQueue(event_queue_t* queue)
{
    list_destruct(&queue->list);
    mutex_destruct(&queue->mutex);
    free(queue);
}

uint8_t event_issue(event_queue_t* destination, EVENT_t type, const void* data, size_t length)
{
    if (destination == 0) // Event handling disabled
    {
        return (1);
    }

    if (destination->num == MAX_EVENTS)
    {
        // Overflow
        dlelement_t* elem = list_alloc_elem(sizeof(event_t) + length, "event (overflow)");
        event_t* ev = elem->data;
        ev->data = 0;
        ev->length = 0;
        ev->type = EVENT_OVERFLOW;
        mutex_lock(&destination->mutex);
        list_append_elem(&destination->list, elem);
        destination->num++;
        mutex_unlock(&destination->mutex);

        return (2);
    }
    else if (destination->num > MAX_EVENTS)
    {
        // Nothing to do. OVERFLOW event has already been added.
        return (3);
    }
    else
    {
        // Add event
        dlelement_t* elem = list_alloc_elem(sizeof(event_t) + length, "event+data");
        event_t* ev = elem->data;
        ev->data = ev+1;
        memcpy(ev->data, data, length);
        ev->length = length;
        ev->type = type;
        mutex_lock(&destination->mutex);
        list_append_elem(&destination->list, elem);
        destination->num++;
        mutex_unlock(&destination->mutex);

        return (0);
    }
}

void event_issueToDisplayedTasks(EVENT_t type, const void* data, size_t length)
{
    for (dlelement_t* e = console_displayed->tasks.head; e != 0; e = e->next)
        event_issue(((task_t*)(e->data))->eventQueue, type, data, length);
}

EVENT_t event_poll(void* destination, size_t maxLength, EVENT_t filter)
{
    task_t* task = currentTask;

    while (task->parent && task->type == THREAD && task->eventQueue == 0)
    {
        task = task->parent; // Use parents eventQueue, if the thread has no own queue.
    }

    if (task->eventQueue == 0 || task->eventQueue->num == 0) // Event handling disabled or no events available
    {
        return (EVENT_NONE);
    }

    dlelement_t* e = 0;
    event_t* ev = 0;
    mutex_lock(&task->eventQueue->mutex);
    if (filter == EVENT_NONE)
    {
        e = task->eventQueue->list.head;
        ev = e ? e->data : 0;
    }
    else
    {
        for (e = task->eventQueue->list.head; e != 0; e = e->next)
        {
            ev = e->data;
            if (ev->type == filter)
                break; // found event
            else
                ev = 0;
        }
    }

    if (ev == 0)
    {
        mutex_unlock(&task->eventQueue->mutex);
        return (EVENT_NONE);
    }

    EVENT_t type = EVENT_NONE;

    if (ev->length > maxLength)
    {
        if (maxLength >= sizeof(size_t)) // Buffer is large enough to store at least the size of the event. Just issue EVENT_BUFFER_TO_SMALL event, leave event in queue.
        {
            *(uint32_t*)destination = ev->length;
            mutex_unlock(&task->eventQueue->mutex);
            return (EVENT_BUFFER_TO_SMALL);
        }

        // data does not fit in pointer - kill the event, return EVENT_BUFFER_TO_SMALL
        type = EVENT_BUFFER_TO_SMALL;
    }
    else
    {
        type = ev->type;
        memcpy(destination, ev->data, ev->length);
    }
    task->eventQueue->num--;
    list_delete(&task->eventQueue->list, e);
    mutex_unlock(&task->eventQueue->mutex);

    return (type);
}

event_t* event_peek(event_queue_t* eventQueue, uint32_t i)
{
    mutex_lock(&eventQueue->mutex);
    dlelement_t* elem = list_getElement(&eventQueue->list, i);
    mutex_unlock(&eventQueue->mutex);

    if (elem == 0)
    {
        return (0);
    }

    return (elem->data);
}

bool event_unlockTask(void* data)
{
    return data && (((event_queue_t*)data)->num > 0);
}

bool waitForEvent(uint32_t timeout)
{
    return (scheduler_blockCurrentTask(BL_EVENT, currentTask->eventQueue, timeout));
}

void event_enable(bool b)
{
    if (b)
    {
        if (currentTask->eventQueue == 0)
        {
            currentTask->eventQueue = event_createQueue();
        }
    }
    else
    {
        if (currentTask->eventQueue)
        {
            event_deleteQueue(currentTask->eventQueue);
            currentTask->eventQueue = 0;
        }
    }
}


/*
* Copyright (c) 2011-2017 The PrettyOS Project. All rights reserved.
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
