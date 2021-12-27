/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cdi/lists.h"
#include "util/list.h"


cdi_list_t cdi_list_create()
{
    return (list_create());
}

void cdi_list_destroy(cdi_list_t list)
{
    list_free(list);
}

cdi_list_t cdi_list_push(cdi_list_t list, void* value)
{
    if (list_append(list, value)) /// TODO: push means prepend, not append
        return list;
    return 0;
}

void* cdi_list_pop(cdi_list_t list)
{
    void* retVal = list->tail->data;
    list_delete(list, list->tail);
    return (retVal);
}

size_t cdi_list_empty(cdi_list_t list)
{
    return (list_isEmpty(list));
}

void* cdi_list_get(cdi_list_t list, size_t index)
{
    dlelement_t* temp = list_getElement(list, index);
    if (temp == 0)
    {
        return (0);
    }
    return (temp->data);
}

cdi_list_t cdi_list_insert(cdi_list_t list, size_t index, void* value)
{
    list_insert(list, list_getElement(list, index), value);
    return (list);
}

void* cdi_list_remove(cdi_list_t list, size_t index)
{
    dlelement_t* elem = list_getElement(list, index);
    void* retVal = elem->data;
    list_delete(list, elem);
    return (retVal);
}

size_t cdi_list_size(cdi_list_t list)
{
    size_t counter = 0;
    for (dlelement_t* elem = list->head; elem; elem = elem->next)
        counter++;
    return (counter);
}

/*
* Copyright (c) 2009-2016 The PrettyOS Project. All rights reserved.
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
