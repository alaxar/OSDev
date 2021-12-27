/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "ring.h"
#include "util.h"
#include "kheap.h"


void ring_construct(ring_t* ring)
{
    ring->current = 0;
    ring->begin   = 0;
}

static void putIn(ring_t* ring, slelement_t* prev, slelement_t* elem)
{
    if (ring->begin == 0) // Ring is empty
    {
        elem->next    = elem;
        ring->begin   = elem;
        ring->current = elem;
    }
    else
    {
        elem->next = prev->next;
        prev->next = elem;
    }
}

static void takeOut(ring_t* ring, slelement_t* prev)
{
    ASSERT(ring);
    if (prev->next == prev) // Just one element in ring
    {
        ring->begin = 0;
        ring->current = 0;
    }
    else
    {
        if (prev->next == ring->begin)   ring->begin   = prev->next->next;
        if (prev->next == ring->current) ring->current = prev->next->next;
        prev->next = prev->next->next;
    }
}

bool ring_insert(ring_t* ring, void* data, bool single)
{
    ASSERT(ring);
    if (single && ring->begin != 0) // check if an element with the same data is already in the ring
    {
        slelement_t* current = ring->current;
        slelement_t* begin = current;
        do
        {
            if (current->data == data)
            {
                return (false);
            }
            current = current->next;
        }
        while (current != begin);
    }
    slelement_t* item = malloc(sizeof(slelement_t), 0, "ring-element");
    if (item)
    {
        item->data = data;
        putIn(ring, ring->current, item);
        return (true);
    }
    return (false);
}

bool ring_isEmpty(const ring_t* ring)
{
    ASSERT(ring);
    return (ring->begin == 0);
}

bool ring_deleteFirst(ring_t* ring, void* data)
{
    ASSERT(ring);
    if (ring->begin == 0) return (false);

    slelement_t* current = ring->current;
    slelement_t* begin = current;
    do
    {
        if (current->next->data == data) // next element will be deleted
        {
            slelement_t* temp = current->next;
            takeOut(ring, current);
            free(temp);
            return (true);
        }
        current = current->next;
    }
    while (current != begin);

    return (false);
}

void ring_move(ring_t* dest, ring_t* source, void* data)
{
    ASSERT(source && dest);
    if (source == 0 || dest == 0 || source->begin == 0) return;

    slelement_t* prev    = source->begin;
    slelement_t* begin   = prev;
    slelement_t* current = prev->next;

    do
    {
        if (current->data == data) // Found. Take it out.
        {
            takeOut(source, prev);
            break;
        }
        prev    = current;
        current = current->next;
    }
    while (prev != begin);

    ASSERT(current->data == data); // Found element.

    // Insert it to dest ring.
    putIn(dest, dest->current, current);
}


/*
* Copyright (c) 2009-2014 The PrettyOS Project. All rights reserved.
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
