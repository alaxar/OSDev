/*
 *  license and disclaimer for the use of this source code as per statement below
 *  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
 */

#include "ipc.h"
#include "tasking/task.h"
#include "util/util.h"
#include "kheap.h"


static ipc_node_t root =
{
    .name    = 0,        .type        = IPC_FOLDER, .data.folder = list_init(), .owner = 0,
    .general = IPC_READ, .accessTable = 0,          .parent      = 0
};
static uint16_t printLineCounter = 0;


// private interface

static ipc_node_t* getNode(const char* remainingPath, ipc_node_t* node)
{
    const char* end = strpbrk(remainingPath, "/|\\");

    if (node->type == IPC_FOLDER)
    {
        if (end == 0) // Final element
        {
            for (dlelement_t* e = node->data.folder.head; e != 0; e = e->next)
            {
                ipc_node_t* newnode = e->data;
                if (strcmp(remainingPath, newnode->name) == 0)
                    return (newnode);
            }
        }
        else
        {
            size_t sublength = end-remainingPath;
            for (dlelement_t* e = node->data.folder.head; e != 0; e = e->next)
            {
                ipc_node_t* newnode = e->data;
                if (strncmp(remainingPath, newnode->name, sublength) == 0 && strlen(newnode->name) == sublength)
                {
                    if (newnode->type == IPC_FOLDER)
                        return (getNode(end+1, newnode)); // One layer downwards
                    else
                        return (0); // Node with name found, but we cannot go one step downwards, because its no folder. -> Not found.
                }
            }
        }
    }

    return (0); // Not found
}

static bool accessAllowed(ipc_node_t* node, IPC_RIGHTS needed)
{
    uint32_t pid = getpid();

    if (pid == 0 || pid == node->owner) // Kernel and owner have full access
        return (true);

    if (node->accessTable)
    {
        for (dlelement_t* e = node->accessTable->head; e != 0; e = e->next)
        {
            ipc_certificate_t* certificate = e->data;
            if (certificate->owner == pid) // Task has a specific certificate to access this node
                return ((certificate->privileges & needed) == needed);
        }
    }

    return ((node->general & needed) == needed); // Use general access rights as fallback
}

static IPC_ERROR prepareNodeToWrite(ipc_node_t** node, const char* path, IPC_TYPE type)
{
    if (*node == 0)
    {
        IPC_ERROR err = ipc_createNode(path, node, type);
        if (err != IPC_SUCCESSFUL)
            return (err);
    }
    else if ((*node)->type != type)
        return (IPC_WRONGTYPE);
    else if (!accessAllowed(*node, IPC_WRITE))
        return (IPC_ACCESSDENIED);

    return (IPC_SUCCESSFUL);
}

static IPC_ERROR createNode(ipc_node_t* parent, ipc_node_t** node, const char* name, IPC_TYPE type)
{
    if (!parent)
        return (IPC_NULLPOINTER);
    if (parent->type != IPC_FOLDER)
        return (IPC_WRONGTYPE);

    *node = malloc(sizeof(ipc_node_t), 0, "ipc_node_t");
    (*node)->name        = strdup(name, "ipc_node_t::name");
    (*node)->type        = type;
    (*node)->owner       = getpid();
    (*node)->general     = IPC_READ;
    (*node)->accessTable = 0;
    (*node)->parent      = parent;
    memset(&(*node)->data, 0, sizeof((*node)->data));

    list_append(&parent->data.folder, *node);

    return (IPC_SUCCESSFUL);
}

static void ipc_printNode(ipc_node_t* node, int indent)
{
    if (printLineCounter >= 37)
    {
        printLineCounter = 0;
        waitForKeyStroke();
    }
    else
        printLineCounter++;

    if (node->name)
    {
        textColor(TEXT);
        putch('\n');
        for (int i = 0; i < indent; i++)
            puts("   ");
        printf("%s", node->name);
        if (node->type != IPC_FOLDER && node->type != IPC_FILE)
            puts(": ");
    }
    textColor(DATA);
    switch (node->type)
    {
        case IPC_INT:
            printf("%i", (int32_t)node->data.integer);
            break;
        case IPC_UINT:
            printf("%u", (uint32_t)node->data.uinteger);
            break;
        case IPC_QWORD:
            if (node->data.uinteger <= 0xFF)
                printf("%yh", (uint8_t)node->data.uinteger);
            else if (node->data.uinteger <= 0xFFFF)
                printf("%xh", (uint16_t)node->data.uinteger);
            else if (node->data.uinteger <= 0xFFFFFFFF)
                printf("%Xh", (uint32_t)node->data.uinteger);
            else
                printf("%X%Xh", (uint32_t)(node->data.uinteger >> 32), (uint32_t)node->data.uinteger);
            break;
        case IPC_SIZE:
            printf("%Sa", node->data.uinteger);
            break;
        case IPC_FLOAT:
            printf("%f", node->data.floatnum);
            break;
        case IPC_STRING:
            printf("%s", node->data.string);
            break;
        case IPC_FILE:
            puts(" (file)");
            break;
        default:
            break;
    }
    textColor(LIGHT_GRAY);
    printf("  o: %u; g: %y", node->owner, node->general);
    if (node->accessTable)
    {
        for (dlelement_t* elem = node->accessTable->head; elem; elem = elem->next)
        {
            ipc_certificate_t* cert = elem->data;
            printf(", %u: %y", cert->owner, cert->privileges);
        }
    }
    textColor(TEXT);

    if (node->type == IPC_FOLDER)
        for (dlelement_t* e = node->data.folder.head; e != 0; e = e->next)
            ipc_printNode(e->data, indent + 1);
}


// Public interface (kernel)

ipc_node_t* ipc_getNode(const char* path)
{
    if (*path == '/' || *path == '|' || *path == '\\')
        path++;
    ipc_node_t* parent = &root;
    if (*path == '~')
    {
        parent = currentTask->ipc_node;
        path += 2;
    }
    else if (*path == '>')
    {
        parent = currentTask->ipc_working;
        path += 2;
    }
    return (getNode(path, parent));
}

IPC_ERROR ipc_createNode(const char* path, ipc_node_t** node, IPC_TYPE type)
{
    if (*path == '/' || *path == '|' || *path == '\\')
        path++;
    ipc_node_t* parent = &root;
    if (*path == '~')
    {
        parent = currentTask->ipc_node;
        path += 2;
    }
    else if (*path == '>')
    {
        parent = currentTask->ipc_working;
        path += 2;
    }

    size_t size = strlen(path)+1;
    char npath[size];
    memcpy(npath, path, size);
    char* nodename;
    char* temp = npath-1;
    do
    {
        nodename = temp+1;
        temp = strpbrk(temp+1, "/|\\");
        if (temp)
            *temp = 0; // insert termination (splitting up string)

        ipc_node_t* nparent = getNode(nodename, parent);
        if (!nparent && temp)
        {
            IPC_ERROR err = createNode(parent, &nparent, nodename, IPC_FOLDER);
            if (err != IPC_SUCCESSFUL)
                return err;
        }
        if (temp)
            parent = nparent;
    } while(temp);

    return (createNode(parent, node, nodename, type));
}

IPC_ERROR ipc_deleteNode(ipc_node_t* node)
{
    if (!accessAllowed(node, IPC_WRITE))
        return (IPC_ACCESSDENIED);

    if (node->type == IPC_FOLDER)
    {
        IPC_ERROR err = IPC_SUCCESSFUL;
        for (dlelement_t* e = node->data.folder.head; e != 0; e = e->next)
        {
            err = ipc_deleteNode(e->data);
        }
        if (err != IPC_SUCCESSFUL)
            return err;
    }

    free(node->name);
    if (node->parent)
        list_delete(&node->parent->data.folder, list_find(&node->parent->data.folder, node));
    if (node->type == IPC_STRING)
        free(node->data.string);
    else if (node->type == IPC_FOLDER)
        list_destruct(&node->data.folder);
    else if (node->type == IPC_FILE)
        ; // TODO
    if (node->accessTable)
    {
        list_free(node->accessTable);
    }

    free(node);

    return (IPC_SUCCESSFUL);
}

void ipc_print(void)
{
    textColor(HEADLINE);
    puts("\nIPC:");
    textColor(TEXT);
    printLineCounter = 0;
    ipc_printNode(&root, -1);
    putch('\n');
}


// Public interface (syscalls)

IPC_ERROR ipc_fopen(const char* path, file_t** destination, const char* mode)
{
    *destination = 0;
    ipc_node_t* node = ipc_getNode(path);
    if (node == 0)
        return (IPC_NOTFOUND);
    if (node->type != IPC_FILE)
        return (IPC_WRONGTYPE);
    if (mode[0] == 'r' && !accessAllowed(node, IPC_READ))
        return (IPC_ACCESSDENIED);
    if ((mode[0] == 'a' || mode[0] == 'w') && !accessAllowed(node, IPC_WRITE))
        return (IPC_ACCESSDENIED);

    *destination = ramdisk_openFile(node->data.file, path, mode);
    return IPC_SUCCESSFUL;
}

IPC_ERROR ipc_getFolder(const char* path, char* destination, size_t length) // TODO: Its possible to solve this more efficient. For example we currently require one byte too much as string length
{
    ipc_node_t* node = ipc_getNode(path);
    if (node == 0)
        return (IPC_NOTFOUND);
    if (node->type != IPC_FOLDER)
        return (IPC_WRONGTYPE);
    if (!accessAllowed(node, IPC_READ))
        return (IPC_ACCESSDENIED);

    // Collect length to check if destination is large enough
    size_t neededLength = 2;
    for (dlelement_t* e = node->data.folder.head; e != 0; e = e->next)
    {
        ipc_node_t* child = e->data;
        neededLength += strlen(child->data.string) + 1;
    }
    if (neededLength > length)
    {
        if (length >= sizeof(size_t))
            *(size_t*)destination = neededLength;
        return (IPC_NOTENOUGHMEMORY);
    }
    destination[0] = 0; // Clear destination
    for (dlelement_t* e = node->data.folder.head; e != 0; e = e->next)
    {
        ipc_node_t* child = e->data;
        strcat(destination, child->name);
        strcat(destination, "|");
    }

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_getString(const char* path, char* destination, size_t* length)
{
    ipc_node_t* node = ipc_getNode(path);
    if (node == 0)
        return (IPC_NOTFOUND);
    if (node->type != IPC_STRING)
        return (IPC_WRONGTYPE);
    if (!accessAllowed(node, IPC_READ))
        return (IPC_ACCESSDENIED);
    size_t requiredLength = strlen(node->data.string)+1;

    size_t providedLength = *length;
    *length = requiredLength;
    if (providedLength == 0)
        return (IPC_SUCCESSFUL);

    if (requiredLength > providedLength || !destination)
        return (IPC_NOTENOUGHMEMORY);

    memcpy(destination, node->data.string, requiredLength);

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_getConstString(const char* path, const char** destination)
{
    ipc_node_t* node = ipc_getNode(path);
    if (node == 0)
        return (IPC_NOTFOUND);
    if (node->type != IPC_STRING)
        return (IPC_WRONGTYPE);
    if (!accessAllowed(node, IPC_READ))
        return (IPC_ACCESSDENIED);

    *destination = node->data.string;

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_getInt(const char* path, int64_t* destination)
{
    ipc_node_t* node = ipc_getNode(path);
    if (node == 0)
        return (IPC_NOTFOUND);
    if (node->type != IPC_INT && node->type != IPC_UINT && node->type != IPC_QWORD && node->type != IPC_SIZE)
        return (IPC_WRONGTYPE);
    if (!accessAllowed(node, IPC_READ))
        return (IPC_ACCESSDENIED);

    *destination = node->data.integer;

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_getDouble(const char* path, double* destination)
{
    ipc_node_t* node = ipc_getNode(path);
    if (node == 0)
        return (IPC_NOTFOUND);
    if (node->type != IPC_FLOAT)
        return (IPC_WRONGTYPE);
    if (!accessAllowed(node, IPC_READ))
        return (IPC_ACCESSDENIED);

    *destination = node->data.floatnum;

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_setString(const char* path, const char* source)
{
    ipc_node_t* node = ipc_getNode(path);

    IPC_ERROR err = prepareNodeToWrite(&node, path, IPC_STRING);
    if (err != IPC_SUCCESSFUL)
        return (err);

    free(node->data.string);
    node->data.string = strdup(source, "ipc string");

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_setInt(const char* path, const int64_t* source, IPC_TYPE type)
{
    if (type != IPC_INT && type != IPC_UINT && type != IPC_QWORD && type != IPC_SIZE)
        return IPC_WRONGTYPE;

    ipc_node_t* node = ipc_getNode(path);

    IPC_ERROR err = prepareNodeToWrite(&node, path, type);
    if (err != IPC_SUCCESSFUL)
        return (err);

    node->data.integer = *source;

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_setDouble(const char* path, const double* source)
{
    ipc_node_t* node = ipc_getNode(path);

    IPC_ERROR err = prepareNodeToWrite(&node, path, IPC_FLOAT);
    if (err != IPC_SUCCESSFUL)
        return (err);

    node->data.floatnum = *source;

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_deleteKey(const char* path)
{
    ipc_node_t* node = ipc_getNode(path);
    if (node == 0)
        return (IPC_NOTFOUND);
    if (!node->parent || !accessAllowed(node->parent, IPC_WRITE)) // Deleting requires write access to nodes parent
        return (IPC_ACCESSDENIED);

    return (ipc_deleteNode(node));
}

IPC_ERROR ipc_setAccess(const char* path, IPC_RIGHTS permissions, uint32_t task)
{
    ipc_node_t* node = ipc_getNode(path);
    if (node == 0)
        return (IPC_NOTFOUND);
    if (!accessAllowed(node, IPC_DELEGATERIGHTS))
        return (IPC_ACCESSDENIED);

    if (task == 0)
        node->general = permissions;
    else
    {
        if (!node->accessTable)
            node->accessTable = list_create();
        for (dlelement_t* e = node->accessTable->head; e != 0; e = e->next)
        {
            ipc_certificate_t* certificate = e->data;
            if (certificate->owner == task) // Task has already specific certificate to access this node
            {
                certificate->privileges = permissions;
                return (IPC_SUCCESSFUL);
            }
        }
        dlelement_t* elem = list_alloc_elem(sizeof(ipc_certificate_t), "ipc_certificate_t");
        ipc_certificate_t* certificate = elem->data;
        certificate->owner = task;
        certificate->privileges = permissions;
        if (node->accessTable == 0)
            node->accessTable = list_create();
        list_append_elem(node->accessTable, elem);
    }

    return (IPC_SUCCESSFUL);
}

IPC_ERROR ipc_setTaskWorkingNode(const char* path, bool create)
{
    ipc_node_t* working = ipc_getNode(path);
    if (!working && create)
        return ipc_createNode(path, &currentTask->ipc_working, IPC_FOLDER);
    currentTask->ipc_working = working;
    return currentTask->ipc_working ? IPC_SUCCESSFUL : IPC_NOTFOUND;
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
