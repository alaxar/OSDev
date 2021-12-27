#ifndef EVENT_LIST_H
#define EVENT_LIST_H

#include "list.h"
#include "tasking/synchronisation.h"


typedef struct todoList
{
    list_t queue;
    mutex_t mutex;
} todoList_t;


todoList_t* todolist_create(void);                           // Allocates memory for a todoList_t and initializes it
void todoList_add(todoList_t* list, void (*function)(void*, size_t), void* data, size_t length, uint32_t executionTime); // Takes a functionpointer
void todoList_execute(todoList_t* list);                 // Executes the content of the queue and clears the queue
void todoList_wait(todoList_t* list);                    // Waits (using scheduler) until there is something to do
bool todoList_unlockTask(void* data);                    // Used for scheduler. Returns true if there are exercises in the list that blocks the task
void todolist_delete(todoList_t* list);                  // Frees memory of a todoList_t


#endif
