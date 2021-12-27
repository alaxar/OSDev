#ifndef RING_H
#define RING_H

#include "os.h"

// single linked list element
typedef struct slelement slelement_t;
struct slelement
{
    void*        data;
    slelement_t* next;
};

typedef struct
{
    slelement_t* begin;
    slelement_t* current;
} ring_t;


#define ring_init() {0, 0}
void    ring_construct(ring_t* ring);                        // Allocates memory for a ring, returns a pointer to it
bool    ring_insert(ring_t* ring, void* data, bool single);  // Inserts an element in the ring at the current position. If single==true then it will be inserted only if its not already in the ring
bool    ring_isEmpty(const ring_t* ring);                    // Returns true, if the ring is empty (data == 0)
bool    ring_deleteFirst(ring_t* ring, void* data);          // Deletes the first element equal to data, returns true if an element has been deleted
void    ring_move(ring_t* dest, ring_t* source, void* data); // Looks for an element with given data in source, takes it out from this ring and inserts it to dest. Instead of doing insert and delete this operations saves memory allocations


#endif
