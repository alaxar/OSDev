/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cdi/mem.h"
#include "util/util.h"
#include "paging.h"
#include "kheap.h"


struct cdi_mem_area* cdi_mem_alloc(size_t size, cdi_mem_flags_t flags) // TODO: Improve, make it conform with specification
{
    size_t alignment = 2U << ((flags & CDI_MEM_ALIGN_MASK) - 1);

    // Wir vergeben nur ganze Pages
    size = (size + 0xFFF) & (~0xFFFU);

    // Wenn die physische Adresse nicht intressiert, koennen wir das Alignment
    // ignorieren.
    if ((flags & CDI_MEM_VIRT_ONLY))
    {
        alignment = 0;
    }

    if (flags & CDI_MEM_PHYS_CONTIGUOUS)
        alignment |= HEAP_CONTINUOUS;

    // Speicher holen
    void* vaddr = malloc(size, alignment, "cdi_mem: vmem");
    if (vaddr == 0)
        return (0);
    uintptr_t paddr = paging_getPhysAddr(vaddr);

    // cdi_mem_area anlegen und befuellen
    struct cdi_mem_sg_item* sg_item = malloc(sizeof(*sg_item), 0, "cdi_mem: sg_item");
    sg_item->size = size;
    sg_item->start = paddr;

    struct cdi_mem_area* area = malloc(sizeof(*area), 0, "cdi_mem: area");
    area->size = size;
    area->vaddr = vaddr;
    area->paddr.num = 1;
    area->paddr.items = sg_item;
    area->osdep.malloced = true;

    return area;
}

struct cdi_mem_area* cdi_mem_map(uintptr_t paddr, size_t size)
{
    size_t offset = paddr % PAGESIZE;
    void* vaddr = paging_allocMMIO(paddr - offset, alignUp(size + offset, PAGESIZE) / PAGESIZE) + offset;

    if (vaddr == 0)
        return (0);

    struct cdi_mem_sg_item* sg_item = malloc(sizeof(*sg_item), 0, "cdi_mem: sg_item");
    sg_item->size = size;
    sg_item->start = paddr;


    struct cdi_mem_area* area = malloc(sizeof(*area), 0, "cdi_mem: area");
    area->size = size;
    area->paddr.items = sg_item;
    area->paddr.num = 1;
    area->vaddr = vaddr;
    area->osdep.malloced = false;

    return (area);
}

void cdi_mem_free(struct cdi_mem_area* area)
{
    if (area->osdep.malloced)
        free(area->vaddr);
    else
        paging_free(kernelPageDirectory, area->vaddr, area->size);
    free(area->paddr.items);
    free(area);
}

struct cdi_mem_area* cdi_mem_require_flags(struct cdi_mem_area* p, cdi_mem_flags_t flags)
{
    struct cdi_mem_area* new = cdi_mem_alloc(p->size, flags);
    if (new == 0)
        return (0);

    memcpy(new->vaddr, p->vaddr, new->size);
    return (new);
}

int cdi_mem_copy(struct cdi_mem_area* dest, struct cdi_mem_area* src)
{
    if (dest->size != src->size)
        return -1;

    if (dest->vaddr != src->vaddr)
        memcpy(dest->vaddr, src->vaddr, dest->size);

    return (0);
}


/*
* Copyright (c) 2009-2015 The PrettyOS Project. All rights reserved.
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
