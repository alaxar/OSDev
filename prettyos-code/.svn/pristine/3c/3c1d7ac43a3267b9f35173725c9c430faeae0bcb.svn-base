/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "todo_list.h"
#include "kheap.h"
#include "util.h"
#include "tasking/scheduler.h"
#include "timer.h"


typedef struct
{
    void*      data;
    size_t     length;
    void     (*function)(void*, size_t);
    uint32_t   timeToExecute;
} todoList_task_t;


todoList_t* todolist_create(void)
{
    todoList_t* list = malloc(sizeof(todoList_t), 0, "todoList");
    list_construct(&list->queue);
    mutex_construct(&list->mutex);
    return (list);
}

void todoList_add(todoList_t* list, void (*function)(void*, size_t), void* data, size_t length, uint32_t executionTime)
{
    dlelement_t* elem = list_alloc_elem(sizeof(todoList_task_t) + length, "todoList_task_t + ::data");
    todoList_task_t* task = elem->data;

    if (length != 0)
    {
        task->data = (void*)(task + 1);
        memcpy(task->data, data, length);
    }
    else
    {
        task->data = 0;
    }

    task->length = length;
    task->timeToExecute = executionTime;
    task->function = function;

    mutex_lock(&list->mutex);
    list_append_elem(&list->queue, elem);
    mutex_unlock(&list->mutex);
}

void todoList_execute(todoList_t* list)
{
    mutex_lock(&list->mutex);
    for (dlelement_t* e = list->queue.head; e != 0;)
    {
        todoList_task_t* task = e->data;

        if (task->timeToExecute == 0 || task->timeToExecute < timer_getMilliseconds())
        {
            task->function(task->data, task->length);

            e = list_delete(&list->queue, e);
        }
        else
        {
            e = e->next;
        }
    }
    mutex_unlock(&list->mutex);
}

void todoList_wait(todoList_t* list)
{
    scheduler_blockCurrentTask(BL_TODOLIST, list, 0);
}

bool todoList_unlockTask(void* data)
{
    return (((todoList_t*)data)->queue.head != 0);
}

void todolist_delete(todoList_t* list)
{
    mutex_destruct(&list->mutex);
    list_destruct(&list->queue);
    free(list);
}


/*
* Copyright (c) 2010-2015 The PrettyOS Project. All rights reserved.
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
