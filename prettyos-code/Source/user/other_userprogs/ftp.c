/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "dns_help.h"


int main(void)
{
    event_enable(true);
    uint8_t buffer[4096];
    EVENT_t ev = event_poll(buffer, 4096, EVENT_NONE);

    size_t waitingDataCommand = 0, renaming = 0, enterPasvMode = 0, fileTransfer = 0, binaryFileTransfer = 0, saveFile = 0, storeFile = 0, fileSize = 0, maxFileSize = 30000;

    uint8_t fileData[maxFileSize];
    char command[200], hostname[100], user[100], pass[100], ctrlPort[10];

    printLine("================================================================================", 0, 0x0B);
    printLine("                                   FTP Client", 1, 0x0B);
    printLine("--------------------------------------------------------------------------------", 3, 0x0B);
    iSetCursor(0, 5);

    printf("Server:\n");
    gets_s(hostname, 100);
    printf("\nUser:\n");
    gets_s(user, 100);
    printf("\nPassword:\n");
    gets_s(pass, 100);
    printf("\nPort:\n");
    gets_s(ctrlPort, 10);

    clearScreen(0x00);
    setScrollField(11, 46);
    printLine("================================================================================", 0, 0x0B);
    printLine("                                   FTP Client", 1, 0x0B);
    printLine("--------------------------------------------------------------------------------", 3, 0x0B);
    iSetCursor(0, 4);
    textColor(0x0F);
    printf("-F1:  Show file\t\t\t\t-F2:  Save file\n");
    printf("-F3:  Upload File\t\t\t-F4:  Delete File\n");
    printf("-F5:  Create Directory\t\t\t-F6:  Remove Directory\n");
    printf("-F7:  Change current directory\t\t-F8:  Get current directory\n");
    printf("-F9:  List files/directories\t\t-F10: Rename file/directory\n");
    printf("-F11: Change file/directory permissions\t-F12: Enter FTP-Command\n");
    printf("--------------------------------------------------------------------------------\n\n");

    IP4_t IP = getAddrByName(hostname), dataIP;
    uint16_t controlPort = atoi(ctrlPort), dataPort;
    uint32_t control = tcp_connect(IP, controlPort), dataConnection = 0;

    printf("\nConnected (ID = %u). Wait until connection is established...\n", control);

    for (;;)
    {
        switch (ev)
        {
            case EVENT_NONE:
                waitForEvent(0);
                break;
            case EVENT_TCP_CONNECTED:
            {
                textColor(0x0A);
                printf("\nESTABLISHED.\n\n");

                tcpConnectedEventHeader_t* header = (void*)buffer;
                if (header->connectionID == dataConnection && waitingDataCommand)
                {
                    waitingDataCommand = 0;
                    tcp_send(control, command, strlen(command));
                }
                break;
            }
            case EVENT_TCP_CLOSED:
            {
                uint32_t connectionID = *(uint32_t*)buffer;
                if (connectionID == dataConnection)
                {
                    textColor(0x07);
                    printf("Closed dataConnection.\n");
                }
                break;
            }
            case EVENT_TCP_RECEIVED:
            {
                tcpReceivedEventHeader_t* header = (void*)buffer;

                if (header->connectionID == dataConnection)
                {
                    if (fileTransfer)
                    {
                        if (binaryFileTransfer)
                        {
                            uint8_t* data = (void*)(header+1);
                            data[header->length] = 0;
                            memcpy(fileData+fileSize, data, header->length);
                            fileSize += header->length;
                        }
                        else
                        {
                            char* data = (void*)(header+1);
                            data[header->length] = 0;
                            memcpy(fileData+fileSize, data, header->length);
                            fileSize += header->length;
                        }
                    }
                    else
                    {
                        char* data = (void*)(header+1);
                        data[header->length] = 0;

                        textColor(0x06);
                        printf("%s\n", data);
                        textColor(0x0F);
                    }
                }
                else if (header->connectionID == control)
                {
                    char* data = (void*)(header+1);
                    data[header->length] = 0;
                    textColor(0x09);
                    printf("%s\n", data);
                    textColor(0x0F);

                    if (data[0] == '1' && data[1] == '5' && data[2] == '0')
                    {
                        if (storeFile)
                        {
                            storeFile = 0;
                            printf("Enter filename/path to load(e.g.:\"1:/test.txt\",max. 15 characters):\n");
                            char filename[15];
                            gets_s(filename, 15);
                            FILE* f = fopen(filename, "r");

                            char c;
                            while (1)
                            {
                                c = fgetc(f);
                                if(c == -1)
                                    break;
                                memcpy(fileData+fileSize, &c, 1);
                                fileSize++;
                            }

                            size_t mod = (fileSize % 1460), packets = ((fileSize - mod) / 1460);
                            size_t i = 0;
                            while (i < packets)
                            {
                                tcp_send(dataConnection, fileData+i*1460, 1460);
                                i++;
                            }
                            if (mod > 0)
                            {
                                tcp_send(dataConnection, fileData+i*1460, mod);
                            }
                            tcp_close(dataConnection);
                            fclose(f);
                        }
                    }
                    else if (data[0] == '2' && data[1] == '0' && data[2] == '0')
                    {
                        if (enterPasvMode)
                        {
                            enterPasvMode = 0;
                            tcp_send(control, "PASV\r\n", 6);
                        }
                    }
                    else if (data[0] == '2' && data[1] == '2' && data[2] == '0')
                    {
                        char pStr[200];
                        snprintf(pStr, 200, "USER %s\r\n", user);
                        tcp_send(control, pStr, strlen(pStr));
                    }
                    else if (data[0] == '2' && data[1] == '2' && data[2] == '6')
                    {
                        if (saveFile)
                        {
                            saveFile = 0;
                            fileTransfer = 0;
                            printf("Save file.\n\nEnter filename/path(e.g.:\"1:/test.txt\",max. 15 characters):\n");
                            char saveFileName[15];
                            gets_s(saveFileName, 15);
                            FILE* f = fopen(saveFileName, "w");
                            if (binaryFileTransfer)
                            {
                                if (fwrite(fileData, fileSize, 1, f))
                                {
                                    printf("\n%s:\nOK.\n\n", saveFileName);
                                }
                            }
                            else
                            {
                                if (fwrite(fileData, fileSize, 1, f))
                                {
                                    printf("\n%s:\nOK.\n", saveFileName);
                                }
                            }
                            fclose(f);
                        }
                        tcp_close(dataConnection);
                    }
                    else if (data[0] == '2' && data[1] == '2' && data[2] == '7')
                    {
                        uint8_t temp[6];
                        uint8_t it = 3;
                        do
                        {
                            if (data[it] == '(')
                                break;
                        } while (it++);
                        for (uint8_t i_start = it+1, i_end = it+1, byte = 0; byte < 6; i_end++)
                        {
                            if (data[i_end] == ')')
                            {
                                temp[byte] = atoi(data+i_start);
                                break;
                            }

                            if (data[i_end] == ',')
                            {
                                data[i_end] = 0;
                                temp[byte] = atoi(data+i_start);
                                i_start = i_end+1;
                                byte++;
                            }
                        }

                        for (int i = 0;i < 4;i++)
                            dataIP.IP4[i] = temp[i];
                        dataPort = temp[4]*256+temp[5];

                        dataConnection = tcp_connect(dataIP, dataPort);
                        printf("Connected (ID = %u)...\n ", dataConnection);
                    }
                    else if (data[0] == '2' && data[1] == '3' && data[2] == '0')
                    {
                        printf("Loggin successful.\n\n");
                        tcp_send(control, "OPTS UTF8 ON\r\n", 14);
                    }
                    else if (data[0] == '3' && data[1] == '3' && data[2] == '1')
                    {
                        char pStr[200];
                        snprintf(pStr, 200, "PASS %s\r\n", pass);
                        tcp_send(control, pStr, strlen(pStr));
                    }
                    else if (data[0] == '3' && data[1] == '5' && data[2] == '0')
                    {
                        if (renaming)
                        {
                            renaming = 0;
                            tcp_send(control, command, strlen(command));
                        }
                    }
                    else if (data[0] == '5' && data[1] == '5' && data[2] == '0')
                    {
                        if (fileTransfer)
                        {
                            saveFile = 0;
                            fileTransfer = 0;
                        }
                    }
                    /*
                    FTP reply codes (http://www.w3.org/Protocols/rfc959/9_References.html):
                        150 File status okay; about to open data connection.
                        200 Command okay.
                        220 Service ready for new user.
                        226 Closing data connection. Requested file action successful (for example, file transfer or file abort).
                        227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
                        230 User logged in, proceed.
                        331 User name okay, need password.
                        350 Requested file action pending further information.
                        550 Requested action not taken. File unavailable (e.g., file not found, no access).
                    */
                }
                break;
            }
            case EVENT_KEY_DOWN:
            {
                textColor(0x0F);
                KEY_t* key = (void*)buffer;
                if (*key == KEY_ESC)
                {
                    tcp_send(control, "QUIT\r\n", 6);
                    tcp_close(control);
                    return (0);
                }
                else if (*key == KEY_F1)
                {
                    waitingDataCommand = 1;
                    enterPasvMode = 1;
                    printf("Get file(ASCII-mode).\nEnter filename:\n");
                    char filename[100];
                    gets_s(filename, 100);
                    memset(command,0,200);
                    strcat(command,"RETR ");
                    strcat(command,filename);
                    strcat(command,"\r\n");
                    putchar('\n');
                    tcp_send(control, "TYPE A\r\n", 8);
                }
                else if (*key == KEY_F2)
                {
                    waitingDataCommand = 1;
                    enterPasvMode = 1;
                    saveFile = 1;
                    fileTransfer = 1;
                    binaryFileTransfer = 0;
                    fileSize = 0;
                    printf("Save file.(max. size: %u)\nEnter filename(server):\n", maxFileSize);
                    char filename[100];
                    gets_s(filename, 100);
                    printf("\nEnter transfer mode(A == ASCII, I == Binary):\n");
                    char mode[1];
                    gets_s(mode, 1);
                    memset(command,0,200);
                    strcat(command,"RETR ");
                    strcat(command,filename);
                    strcat(command,"\r\n");
                    memset(fileData, 0, 20000);
                    putchar('\n');
                    if (mode[0] == 'I')
                    {
                        binaryFileTransfer = 1;
                        tcp_send(control, "TYPE I\r\n", 8);
                    }
                    else
                        tcp_send(control, "TYPE A\r\n", 8);
                }
                else if (*key == KEY_F3)
                {
                    waitingDataCommand = 1;
                    enterPasvMode = 1;
                    storeFile = 1;
                    fileSize = 0;
                    printf("Store file(ASCII-mode).\n\nEnter filename(server):\n");
                    char filename[100];
                    gets_s(filename, 100);
                    memset(command,0,200);
                    strcat(command,"STOR ");
                    strcat(command,filename);
                    strcat(command,"\r\n");
                    putchar('\n');
                    tcp_send(control, "TYPE A\r\n", 8);
                }
                else if (*key == KEY_F4)
                {
                    //Delete file
                    printf("Delete file.\nEnter filename:\n");
                    char filename[100];
                    gets_s(filename, 100);
                    memset(command,0,200);
                    strcat(command,"DELE ");
                    strcat(command,filename);
                    strcat(command,"\r\n");
                    putchar('\n');
                    tcp_send(control, command, strlen(command));
                }
                else if (*key == KEY_F5)
                {
                    //Create directory
                    printf("Create directory.\nEnter directory name:\n");
                    char dirname[100];
                    gets_s(dirname, 100);
                    memset(command,0,200);
                    strcat(command,"MKD ");
                    strcat(command,dirname);
                    strcat(command,"\r\n");
                    putchar('\n');
                    tcp_send(control, command, strlen(command));
                }
                else if (*key == KEY_F6)
                {
                    //Remove directory
                    printf("Remove directory.\nEnter directory name:\n");
                    char filename[100];
                    gets_s(filename, 100);
                    memset(command,0,200);
                    strcat(command,"RMD ");
                    strcat(command,filename);
                    strcat(command,"\r\n");
                    putchar('\n');
                    tcp_send(control, command, strlen(command));
                }
                else if (*key == KEY_F7)
                {
                    //Change current directory
                    printf("Change current directory.\n\nEnter directory name(without initiating / !):\n");
                    char dirname[100];
                    gets_s(dirname, 100);
                    memset(command,0,200);
                    strcat(command,"CWD ");
                    strcat(command,dirname);
                    strcat(command,"\r\n");
                    putchar('\n');
                    tcp_send(control, command, strlen(command));
                }
                else if (*key == KEY_F8)
                {
                    //Get current directory
                    printf("Current directory is:\n\n");
                    tcp_send(control, "PWD\r\n", 5);
                }
                else if (*key == KEY_F9)
                {
                    //List files/directories
                    waitingDataCommand = 1;
                    enterPasvMode = 1;
                    memset(command,0,200);
                    strcat(command,"LIST -a\r\n");
                    printf("List files/directories.\n\n");
                    tcp_send(control, "TYPE A\r\n", 8);
                }
                else if (*key == KEY_F10)
                {
                    //Rename
                    renaming = 1;
                    printf("Rename.\nEnter current filename:\n");
                    char oldFilename[100];
                    gets_s(oldFilename, 100);
                    printf("\nEnter new filename:\n");
                    char newFilename[100];
                    gets_s(newFilename, 100);
                    memset(command,0,200);
                    strcat(command,"RNTO ");
                    strcat(command,newFilename);
                    strcat(command,"\r\n");
                    char tempCommand[200];
                    snprintf(tempCommand, 200, "RNFR %s\r\n", oldFilename);
                    putchar('\n');
                    tcp_send(control, tempCommand, strlen(tempCommand));
                }
                else if (*key == KEY_F11)
                {
                    printf("Change file/directory permissions.\n\nEnter file/directory:\n");
                    char filename[183];
                    gets_s(filename, 183);
                    printf("\nEnter new file/directory permissions (e.g. 644):\n");
                    char permissions[3];
                    gets_s(permissions, 3);
                    putchar('\n');
                    memset(command,0,200);
                    snprintf(command, 200, "SITE CHMOD %s %s\r\n", permissions, filename);
                    tcp_send(control, command, strlen(command));
                }
                else if (*key == KEY_F12)
                {
                    printf("Enter command:\n");
                    memset(command,0,200);
                    gets_s(command, 200);
                    strcat(command,"\r\n");
                    tcp_send(control, command, strlen(command));
                }
                break;
            }
            default:
                break;
        }
        ev = event_poll(buffer, 4096, EVENT_NONE);
    }

    tcp_close(control);
    return (0);
}


/*
* Copyright (c) 2011-2015 The PrettyOS Project. All rights reserved.
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