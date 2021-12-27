/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#define static // helps tp prevent #PF, when booted with inserted usb-stick at xhci port

#define MAX_CHAR_PER_LINE 75
#define ENTRY_CACHE_SIZE 10

void eraseFirst(char* string)
{
    strcpy(string, string+1);
}

void prependStr(char* string, char* ins)
{
    size_t toBeInserted = strlen(ins);
    memmove(string+toBeInserted, string, strlen(string));
    memcpy(string, ins, toBeInserted);
}

void prependChar(char* string, char ins)
{
    memmove(string+1, string, strlen(string));
    *string = ins;
}

static unsigned int cursorPos;
static unsigned int entryLength = 0;

static char RenderBuffer[83];
static void drawEntry(const char* entry)
{
    sprintf(RenderBuffer, "$> %s", entry);
    size_t length = min(80, entryLength+3);
    if (length < 80)
        memset(RenderBuffer+length, ' ', 80-length);
    RenderBuffer[80] = 0;
    if (cursorPos < entryLength)
    {
        prependStr(RenderBuffer+3+cursorPos, "%v");
    }
    printLine(RenderBuffer, 40, 0x0D);
}

static char* formatExecutablePath(char** opath)
{
    bool insertELF = false;
    size_t length = strlen(*opath) + 1;
    char* pointpos = strrchr(*opath, '.');
    if (pointpos == 0 || strncicmp(pointpos + 1, "ELF", 3) != 0)
    {
        length += 4;
        insertELF = true;
    }

    char* npath = calloc(length, 1);
    char* retval = npath;
    strcpy(npath, *opath);
    npath += strlen(*opath);
    free(*opath);
    *opath = retval;
    if (insertELF)
    {
        strcpy(npath, ".ELF");
    }
    return (retval);
}


static char* formatPath(char* opath)
{
    return opath;
}

static void printWorkingDir(void)
{
    char* path = getWorkingDir();
    if (path)
    {
        printf("%s\n", path);
        free(path);
    }
}


int main(void)
{
    setScrollField(0, 39);
    event_enable(true);
    char entry[MAX_CHAR_PER_LINE+1];
    char entryCache[ENTRY_CACHE_SIZE][MAX_CHAR_PER_LINE+1] = {{0}};
    int curEntry = -1;
    bool insertMode = false;

    while (true)
    {
        textColor(0x0F);
        entryLength = 0; cursorPos = 0;
        memset(entry, 0, MAX_CHAR_PER_LINE+1);
        drawEntry(entry);

        while (true)
        {
            union {char buffer[4]; KEY_t key;} buffer; // We are only interested in TEXT_ENTERED and KEY_DOWN events. They send 4 bytes at maximum
            waitForEvent(0);
            EVENT_t ev = event_poll(buffer.buffer, 4, EVENT_NONE);

            switch (ev)
            {
                case EVENT_TEXT_ENTERED:
                {
                    if (keyPressed(KEY_ESC) || keyPressed(KEY_LCTRL) || keyPressed(KEY_RCTRL)) // To avoid conflicts with strg/esc shortcuts in kernel
                        break;

                    showInfo((getCurrentSeconds() / 20) % 2 + 1);

                    unsigned char text = buffer.buffer[0];
                    if (text >= 0x20 && (entryLength<MAX_CHAR_PER_LINE || (insertMode && entryLength <= MAX_CHAR_PER_LINE && cursorPos < entryLength)))
                    {
                        if (curEntry != -1)
                        {
                            strcpy(entry, entryCache[curEntry]);
                            curEntry = -1;
                        }
                        if (insertMode)
                        {
                            entry[cursorPos]=text;
                            if (cursorPos == entryLength)
                            {
                                entryLength++;
                            }
                        }
                        else
                        {
                            prependChar(entry+cursorPos, text);
                            ++entryLength;
                        }
                        ++cursorPos;
                    }
                    break;
                }
                case EVENT_KEY_DOWN:
                {
                    if (keyPressed(KEY_ESC) || keyPressed(KEY_LCTRL) || keyPressed(KEY_RCTRL)) // To avoid conflicts with strg/esc shortcuts in kernel
                        break;

                    showInfo((getCurrentSeconds()/20) % 2 + 1);
                    switch (buffer.key)
                    {
                        case KEY_BACK:
                            if (cursorPos > 0)
                            {
                                if (curEntry != -1)
                                {
                                    strcpy(entry, entryCache[curEntry]);
                                    curEntry = -1;
                                }
                                eraseFirst(entry+cursorPos-1);
                                --entryLength;
                                --cursorPos;
                            }
                            break;
                        case KEY_ENTER:
                            if (*(curEntry == -1 ? entry : entryCache[curEntry]) == 0)
                            {
                                break; // entry is empty
                            }
                            cursorPos = entryLength+1;
                            entry[entryLength]='\0';
                            if (curEntry == -1)
                            {
                                //Insert entry
                                for (int i = ENTRY_CACHE_SIZE-2; i >= 0; i--)
                                {
                                    strncpy(entryCache[i+1], entryCache[i], MAX_CHAR_PER_LINE);
                                }
                                strcpy(entryCache[0], entry);
                            }
                            else
                            {
                                //Move entry to front
                                strcpy(entry, entryCache[curEntry]);
                                for (int i = curEntry-1; i >= 0; i--)
                                {
                                    strcpy(entryCache[i+1], entryCache[i]);
                                }
                                strcpy(entryCache[0], entry);
                                curEntry = -1;
                            }
                            textColor(0x03);
                            printf("\n$> %s <--\n", entry);
                            textColor(0x0F);
                            goto EVALUATION;
                            break;
                        case KEY_INS:
                            insertMode = !insertMode;
                            break;
                        case KEY_DEL:
                            if (cursorPos < entryLength)
                            {
                                if (curEntry != -1)
                                {
                                    strcpy(entry, entryCache[curEntry]);
                                    curEntry = -1;
                                }
                                eraseFirst(entry+cursorPos);
                                --entryLength;
                            }
                            break;
                        case KEY_HOME:
                            cursorPos = 0;
                            break;
                        case KEY_END:
                            cursorPos = entryLength;
                            break;
                        case KEY_ARRL:
                            if (cursorPos > 0)
                            {
                                cursorPos--;
                            }
                            break;
                        case KEY_ARRR:
                            if (cursorPos < entryLength)
                            {
                                cursorPos++;
                            }
                            break;
                        case KEY_ARRU:
                            if (curEntry < ENTRY_CACHE_SIZE-1 && *entryCache[curEntry+1] != 0)
                            {
                                ++curEntry;
                                entryLength = strlen(entryCache[curEntry]);
                                cursorPos = entryLength;
                            }
                            break;
                        case KEY_ARRD:
                            if (curEntry >= 0)
                            {
                                --curEntry;
                                if (curEntry == -1)
                                {
                                    entryLength = strlen(entry);
                                }
                                else
                                {
                                    entryLength = strlen(entryCache[curEntry]);
                                }
                                cursorPos = entryLength;
                            }
                            else
                            {
                                memset(entry, 0, MAX_CHAR_PER_LINE);
                                cursorPos = 0;
                                entryLength = 0;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                }
                default:
                    break;
            } //switch
            drawEntry((curEntry == -1 ? entry : entryCache[curEntry]));
        } //while

EVALUATION: // evaluation of entry
        {
            // Arguments
            size_t argc = 1;
            bool apostroph = false;
            // Find out argc
            for (size_t i = 0; entry[i] != 0; i++)
            {
                if (entry[i] == '"')
                    apostroph = !apostroph;

                if (entry[i] == ' ' && !apostroph) // argument end
                    argc++;
            }

            char* argv[argc];
            char* argstart = entry;
            size_t j = 0;
            for (size_t i = 0; entry[i] != 0; i++)
            {
                if (entry[i] == '"')
                    apostroph = !apostroph;

                if (entry[i] == ' ' && !apostroph) // argument end
                {
                    entry[i] = 0;
                    argv[j] = argstart;
                    argstart = entry + i + 1;
                    j++;
                }
            }
            argv[j] = argstart;

            // Duplicate argv[0] - this is required for formatExecutablePath
            size_t entrysize = strlen(argv[0]) + 1;
            char* temp = malloc(entrysize);
            memcpy(temp, argv[0], entrysize);
            argv[0] = temp;

            // Command
            if (strcmp(argv[0], "help") == 0 || strcmp(argv[0], "?") == 0)
            {
                textColor(0x0D);
                puts(" => Implemented Instructions:\n");
                textColor(0x0E);
                puts("   => help, ?    => Displays this help text\n");
                puts("   => dir, ls    => Displays directory contents\n");
                puts("   => chdir, cd  => Change or print current working directory\n");
                puts("   => pwd        => Print current working directory\n");
                puts("   => mkdir, md  => Creates directory\n");
                puts("   => rmdir, rd  => Deletes directory\n");
                puts("   => rm, del    => Deletes file\n");
                puts("   => mv, rename => Renames file\n");
                puts("   => cp, copy   => Copies file\n");
                puts("   => format     => Formats a partition\n");
                puts("   => reboot     => Reboots the system\n");
                puts("   => standby    => Puts the system to standby\n");
                puts("   => shutdown   => Shuts down the system\n");
            }
            else if (strcmp(argv[0], "ls") == 0 || strcmp(argv[0], "dir") == 0)
            {
                if (argc == 1)
                {
                    // Use current working directory
                    char* workingDir = getWorkingDir();
                    floppy_dir(workingDir);
                    free(workingDir);
                }
                else if (argc == 2)
                    floppy_dir(formatPath(argv[1]));
                else
                    puts("Less than two arguments required.");
            }
            else if (strcmp(argv[0], "mkdir") == 0 || strcmp(argv[0], "md") == 0)
            {
                if (argc == 2)
                {
                    folder_t* folder = folderAccess(formatPath(argv[1]), FOLDER_CREATE);
                    folderClose(folder);
                }
                else
                    puts("One argument required.");
            }
            else if (strcmp(argv[0], "rmdir") == 0 || strcmp(argv[0], "rd") == 0)
            {
                if (argc == 2)
                {
                    folder_t* folder = folderAccess(formatPath(argv[1]), FOLDER_DELETE);
                    folderClose(folder);
                }
                else
                    puts("One argument required.");
            }
            else if (strcmp(argv[0], "rm") == 0 || strcmp(argv[0], "del") == 0)
            {
                if (argc == 2)
                    remove(formatPath(argv[1]));
                else
                    puts("One argument required.");
            }
            else if (strcmp(argv[0], "rename") == 0 || strcmp(argv[0], "mv") == 0)
            {
                if (argc == 3)
                    fmove(formatPath(argv[1]), formatPath(argv[2]), false);
                else
                    puts("Two arguments required.");
            }
            else if (strcmp(argv[0], "copy") == 0 || strcmp(argv[0], "cp") == 0)
            {
                if (argc == 3)
                    fmove(formatPath(argv[1]), formatPath(argv[2]), true);
                else
                    puts("Two arguments required.");
            }
            else if (strcmp(argv[0], "cd") == 0 || strcmp(argv[0], "chdir") == 0)
            {
                if (argc == 1)
                    printWorkingDir();
                else if (argc == 2)
                {
                    if (!changeWorkingDir(argv[1]))
                        puts("Failed to change current directory.");
                }
                else
                    puts("No or one argument required.");
            }
            else if (strcmp(argv[0], "pwd") == 0)
            {
                printWorkingDir();
            }
            else if (strcmp(argv[0], "format") == 0)
            {
                getchar(); // Catch RETURN/ENTER

                textColor(0x0E);
                puts("Please enter the partition path (for example: 'A:0:'): ");
                char part[20];
                gets_s(part, 20);
                puts("Please enter the filesystem type (1: FAT12, 2: FAT16, 3: FAT32): ");
                char type[20];
                gets_s(type, 20);
                puts("Please enter the volume label: ");
                char label[20];
                gets_s(label, 20);
                putchar('\n');
                FS_ERROR error = partition_format(part, atoi(type), label);
                if (error != CE_GOOD)
                    printf("\nError: %u", error);
            }
            else if (strcmp(argv[0], "reboot") == 0)
            {
                systemControl(REBOOT);
            }
            else if (strcmp(argv[0], "standby") == 0)
            {
                systemControl(STANDBY);
            }
            else if (strcmp(argv[0], "shutdown") == 0)
            {
                systemControl(SHUTDOWN);
            }
            else
            {
                textColor(0x0D);
                puts(" Looking for file...\n");
                textColor(0x0E);

                FS_ERROR error = execute(formatPath(argv[0]), argc, argv, true);
                switch (error)
                {
                    case CE_GOOD:
                        textColor(0x0A);
                        puts("   => Successful\n");
                        break;
                    case CE_INVALID_FILENAME:
                        textColor(0x0C);
                        puts("   => The path was malformed\n");
                        break;
                    case CE_FILE_NOT_FOUND:
                        error = execute(formatExecutablePath(&argv[0]), argc, argv, true);
                        if (error != CE_GOOD)
                        {
                            textColor(0x0C);
                            puts("   => File not found\n");
                        }
                        else
                        {
                            textColor(0x0A);
                            puts("   => Successful\n");
                        }
                        break;
                    default:
                        textColor(0x0C);
                        printf("   => File load was not successful. Error Code: %i\n", error);
                        break;
                }
            }

            free(argv[0]);
        }
    } //while
    return (0);
}

/*
* Copyright (c) 2009-2017 The PrettyOS Project. All rights reserved.
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
