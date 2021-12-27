/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"


int main(void)
{
    setScrollField(5, 46);
    printLine("================================================================================", 0, 0x0D);
    printLine("                                 Pretty WebShell", 2, 0x0F);
    printLine("--------------------------------------------------------------------------------", 4, 0x0D);

    event_enable(true);
    char buffer[4096];
    EVENT_t ev = event_poll(buffer, 4096, EVENT_NONE);


    iSetCursor(0, 7);
    textColor(0x0F);

    putchar('\n');
    printf(" => Initializing...                                 ");


    textColor(0x07);
    putchar('[');
    textColor(0x0A);
    printf("OK");
    textColor(0x07);
    printf("]\n");
    textColor(0x0F);

    printf(" => Connecting...                                   ");

    IP4_t IP = {.iIP4 = 0};
    uint32_t connection = tcp_connect(IP, 80);

    textColor(0x07);
    putchar('[');
    textColor(0x0A);
    printf("OK");
    textColor(0x07);
    printf("]\n");
    textColor(0x0F);

    printf(" => Listening...                                    ");

    textColor(0x07);
    putchar('[');
    textColor(0x0A);
    printf("OK");
    textColor(0x07);
    printf("]\n");

    printf("\n\n");
    textColor(0x0E);
    printf(" To use this app, simply connect to PrettyOS with a webbrowser of your choice.\n\n");
    textColor(0x0C);
    printf(" Just enter http://Pre.tty.OS.IP:80/");

    printf("\n\n\n");

    textColor(0x09);
    printf(" --------------------------------------------------------");
    textColor(0x0F);
    printf("\n\n");

    for (;;)
    {
        switch (ev)
        {
            case EVENT_NONE:
                waitForEvent(0);
                break;
            case EVENT_TCP_CONNECTED:
            {
                printf(" => Connected - awaiting data...                    ");
                break;
            }
            case EVENT_TCP_RECEIVED:
            {
                textColor(0x07);
                putchar('[');
                textColor(0x0A);
                printf("OK");
                textColor(0x07);
                printf("]\n");
                textColor(0x0F);

                tcpReceivedEventHeader_t* header = (void*)buffer;
                char* data = (void*)(header+1);
                data[header->length] = 0;

                //printf("length: %u", header->length);


                char reqstr[5000] = {0};
                strcpy(reqstr,data);


                *strstr(reqstr, "HTTP/1.") = 0;

                //printf("\n\nRequested:\n%s\n\n",reqstr);

                uint8_t answer = 0;
                uint16_t httpstatuscode = 200;
                uint8_t action = 0;
                /*
                 0:   Nothing
                 10:  Reboot
                 11:  Shutdown
                 20:  Beep
                 30:  Open starwars.ELF
                 */

                //**********************************************************#);
                printf(" => Generating basic HTML code...                   ");

                char bstr[8000];

                strcpy(bstr,"<!DOCTYPE HTML>\n");
                strcat(bstr,"<html>\n");
                strcat(bstr,"<head>\n");
                strcat(bstr,"<title>PrettyOS RemoteControl (PrettyControl)</title>\n");

                strcat(bstr,"<style type=\"text/css\">\n");

                strcat(bstr,"* {\n");
                strcat(bstr,"margin:0px;\n");
                strcat(bstr,"padding:0px;\n");
                strcat(bstr,"font-family: Helvetica, Arial, sans-serif;\n");
                strcat(bstr,"color:#FFFFFF;");
                strcat(bstr,"}\n");

                strcat(bstr,"body {\n");
                strcat(bstr,"background-color:#000000;");
                strcat(bstr,"}\n");

                strcat(bstr,"td {\n");
                strcat(bstr,"background-color:#111111;");
                strcat(bstr,"}\n");

                strcat(bstr,"#content {\n");
                strcat(bstr,"background-color:#333333;\n");
                strcat(bstr,"width:800px;\n");
                strcat(bstr,"min-height:600px;\n");
                strcat(bstr,"margin-top:50px;\n");
                strcat(bstr,"margin-left:auto;\n");
                strcat(bstr,"margin-right:auto;\n");
                strcat(bstr,"padding:10px;\n");
                strcat(bstr,"}\n");

                strcat(bstr,"</style>\n");
                strcat(bstr,"<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n");
                strcat(bstr,"\n");
                strcat(bstr,"</head>\n");
                strcat(bstr,"<body><div id=\"content\">\n");
                strcat(bstr,"<h1>PrettyControl</h1>\n");
                strcat(bstr,"<br>\n");

                textColor(0x07);
                putchar('[');
                textColor(0x0A);
                printf("OK");
                textColor(0x07);
                printf("]\n");
                textColor(0x0F);

                if (strcmp(reqstr,"GET / ") == 0) {
                    //**********************************************************#);
                    printf(" => Requested ");
                    textColor(0x0D);
                    printf("home document");
                    textColor(0x0F);
                    printf(" - generating...         ");
                    answer = 1;


                    strcat(bstr,"<p>Here you can remote-control PrettyOS:</p>\n");
                    strcat(bstr,"<br>\n");
                    strcat(bstr,"<table border=\"\" style=\"width:100%;text-align:center;height:130px;border-spacing:5px;\"><tr style=\"height:30px;\">\n");
                    strcat(bstr,"<td><b>System control</b></td><td><b>Utilities/Entertainment</b></td></tr><tr style=\"height:100px;\">\n");
                    strcat(bstr,"<td>");
                    strcat(bstr,"<a href=\"/reboot\">Reboot PrettyOS</a><br><br>");
                    strcat(bstr,"<a href=\"/shutdown\">Shut down PrettyOS</a>");
                    strcat(bstr,"</td>\n");
                    strcat(bstr,"<td>");
                    strcat(bstr,"<a href=\"/beep\">Make a sound (BEEP)</a><br><br>");
                    strcat(bstr,"<a href=\"/open_starwars\">Open starwars.ELF in PrettyOS</a>");
                    strcat(bstr,"</td>\n\n");
                    strcat(bstr,"\n\n\n");
                    strcat(bstr,"</tr></table>\n");

                    strcat(bstr,"<br><br><br><br><hr><br>\n");
                    strcat(bstr,"<h2>System-status:</h2><br>\n");
                    strcat(bstr,"<ul style=\"margin-left:20px; padding-left:20px;\">\n");

                    strcat(bstr,"<li>rctl-PID: ");
                    char tempstr[20];
                    uint64_t pid = 0;
                    ipc_getInt("~|pid", (int64_t*)&pid);
                    itoa(pid, tempstr);
                    strcat(bstr,tempstr);
                    strcat(bstr,"</li>\n");

                    strcat(bstr,"\n\n\n");
                    strcat(bstr,"</ul>\n");
                    strcat(bstr,"\n\n\n\n");


                } else {
                    if (strcmp(reqstr,"GET /reboot ") == 0) {
                        //**********************************************************#);
                        printf(" => Requested ");
                        textColor(0x0D);
                        printf("reboot");
                        textColor(0x0F);
                        printf(" - generating...                ");

                        answer = 1;
                        action = 10;

                        strcat(bstr,"<p>PrettyOS is rebooting. Please reopen 'rctl.ELF' in PrettyOS and click <a href=\"/\">here</a>.</p>\n");



                    } else {
                        if (strcmp(reqstr,"GET /beep ") == 0) {
                            //**********************************************************#);
                            printf(" => Requested ");
                            textColor(0x0D);
                            printf("beep");
                            textColor(0x0F);
                            printf(" - generating...                  ");
                            answer = 1;
                            action = 20;
                            httpstatuscode = 404;

                            strcat(bstr,"<p>PrettyOS made a sound. Click <a href=\"/\">here</a> to return to the home document.</p>\n");


                        } else {
                            if (strcmp(reqstr,"GET /shutdown ") == 0) {
                                //**********************************************************#);
                                printf(" => Requested ");
                                textColor(0x0D);
                                printf("shutdown");
                                textColor(0x0F);
                                printf(" - generating...              ");
                                answer = 1;
                                action = 11;

                                strcat(bstr,"<p>PrettyOS is shutting down.</p>\n");

                            } else {

                                if (strcmp(reqstr,"GET /open_starwars ") == 0) {
                                    //**********************************************************#);
                                    printf(" => Requested ");
                                    textColor(0x0D);
                                    printf("starwars");
                                    textColor(0x0F);
                                    printf(" - generating...              ");
                                    answer = 1;
                                    action = 30;

                                    strcat(bstr,"<p>PrettyOS opened starwars.ELF. Click <a href=\"/\">here</a>.</p>\n");


                                } else {
                                    textColor(0x0C);
                                    printf(" => UNKNOWN REQUEST: %s\n",reqstr);
                                    //**********************************************************#);
                                    printf(" ===> Generating 500...                             ");
                                    textColor(0x0F);

                                    answer = 1;

                                    strcat(bstr,"<h2>Error 500 (Internal Server Error):</h2><p>Your browser sent a request this server does not understand.</p>");
                                }
                            }
                        }
                    }
                }


                if (answer==1) {
                    strcat(bstr,"<br><br><br><br><br><br><hr><br><p><b>Please note:</b><br>Even though this document is dynamically created by a C-Application, it generates completely valid HTML5-Code!</p></div></body>\n");
                    strcat(bstr,"</html>\n\n");



                    char astr[1000] = {0};

                    if (httpstatuscode==200) {
                        strcat(astr,"HTTP/1.1 200 OK\r\n");
                    } else {
                        if (httpstatuscode==404) {
                            strcat(astr,"HTTP/1.1 500 Internal Server Error\r\n");
                        }
                    }

                    strcat(astr,"Content-Length: ");

                    char length[50];
                    utoa(strlen(bstr), length);
                    strcat(astr,length);
                    strcat(astr,"\r\n");

                    strcat(astr,"Content-Language: en\r\n");
                    strcat(astr,"Keep-Alive: timeout=3, max=1\r\n");
                    strcat(astr,"Content-Type: text/html; charset=utf-8\r\n");
                    strcat(astr,"Connection: close\r\n");

                    char rstr[10000];
                    snprintf(rstr, 10000, "%s\r\n%s", astr, bstr);

                    textColor(0x07);
                    putchar('[');
                    textColor(0x0A);
                    printf("OK");
                    textColor(0x07);
                    printf("]\n");
                    textColor(0x0F);



                    //textColor(0x0A);
                    //puts(rstr);

                    //**********************************************************#);

                    printf(" => ");
                    textColor(0x0B);
                    printf("Sending document");
                    textColor(0x0F);
                    printf("...                             ");
                    textColor(0x0F);

                    uint32_t millis = getCurrentMilliseconds();

                    tcp_send(connection, rstr, strlen(rstr));

                    millis = (getCurrentMilliseconds() - millis);

                    textColor(0x07);
                    putchar('[');
                    textColor(0x0A);
                    printf("OK");
                    textColor(0x07);
                    printf("]\n");
                    textColor(0x0F);

                    putchar('\n');
                    printf(" => Statistics:\n");
                    printf("  -> Document-length was %u bytes.\n",strlen(rstr));
                    printf("  -> It took %u millisecond(s) to send\n",millis);
                    printf("       the document.\n");

                }




                //**********************************************************#);
                putchar('\n');
                printf(" => Closing connection...                           ");
                tcp_close(connection);
                textColor(0x07);
                putchar('[');
                textColor(0x0A);
                printf("OK");
                textColor(0x07);
                printf("]\n");
                textColor(0x0F);


                if (action==10) {
                    systemControl(REBOOT);
                }

                if (action==11) {
                    systemControl(SHUTDOWN);
                }

                if (action==20) {
                    beep(1500,1000);
                }

                if (action==30) {

                    char entry[50];
                    strcpy(entry,"starwars");

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

                    char** argv = malloc(sizeof(char*)*argc);
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
                            argstart = entry+i+1;
                            j++;
                        }
                    }
                    argv[j] = argstart;

                    execute(argv[0], argc, argv, true);

                    free(argv[0]);
                    free(argv);
                }


                //**********************************************************#);
                printf(" => Opening new connection...                       ");
                connection = tcp_connect(IP, 80);
                textColor(0x07);
                putchar('[');
                textColor(0x0A);
                printf("OK");
                textColor(0x07);
                printf("]\n");

                putchar('\n');
                textColor(0x09);
                //***************************************************************#);
                printf(" --------------------------------------------------------");
                textColor(0x0F);

                printf("\n\n");
                break;
            }
            case EVENT_KEY_DOWN:
            {
                KEY_t* key = (void*)buffer;
                if (*key == KEY_ESC)
                {
                    tcp_close(connection);
                    return (0);
                }
                break;
            }
            default:
                break;
        }
        ev = event_poll(buffer, 4096, EVENT_NONE);
    }

    tcp_close(connection);
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
