/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "string.h"
#include "stdio.h"
#include "ctype.h"
#include "stdlib.h"
#include "math.h"


static void replace(const char* source, char* dest, int Pos, int length, const char* toBeInserted) {
    strncpy(dest, source, Pos);
    strcpy(dest+Pos, toBeInserted);
    strcat(dest, source+Pos+length);
}

static int getPrevNumber(size_t Pos, const char* string) {
    int i;
    for (i = Pos-1; i >= 0 && string[i] != '('; i--) {
        if (!isdigit(string[i])) {
            return (atoi(string+i+1));
        }
    }
    if(string[i] != '(' && isdigit(*string))
        return(atoi(string));
    else if(string[i] == '(' && isdigit(string[i+1]))
        return(atoi(string+i+1));
    else
        return(0);
}
static size_t getPrevNumberPos(size_t Pos, const char* string) {
    for (int i = Pos-1; i >= 0 && string[i] != '('; i--) {
        if (!isdigit(string[i])) {
            return (i+1);
        }
    }
    return (0);
}
static int getNextNumber(size_t Pos, const char* string) {
    int i = Pos+1;
    if(string[i] == '-')
        i++;
    for (;; i++) {
        if (!isdigit(string[i])) {
            return (atoi(string+Pos+1));
        }
    }
}
static size_t getNextNumberPos(size_t Pos, const char* string) {
    size_t i = Pos+1;
    if(string[i] == '-')
        i++;
    for (;; i++) {
        if (!isdigit(string[i])) {
            return (i-1);
        }
    }
}

static int find_first(const char* string, const char* search) {
    for (int i = 0; string[i] != 0 && string[i] != ')'; i++) {
        for (int j = 0; search[j] != 0; j++) {
            if (string[i] == search[j]) {
                return (i);
            }
        }
    }
    return (-1);
}

static void SolvePow(char* term, size_t point)
{
    int32_t erg = 0;
    int num = getPrevNumber(point, term);
    size_t length;
    if (term[point] == '²') {
        erg = num*num;
        length = point - getPrevNumberPos(point, term) + 1;
    }
    else {
        erg = pow(num, getNextNumber(point, term));
        length = getNextNumberPos(point, term) - getPrevNumberPos(point, term)+1;
    }
    char temp[32];
    itoa(erg, temp);
    size_t strlength = strlen(term)+1;
    char temp2[strlength];
    memcpy(temp2, term, strlength);
    replace(temp2, term, getPrevNumberPos(point, term), length, temp);
}
static void SolveMul(char* term, size_t point)
{
    int32_t erg = 0;
    if (term[point] == '*') {
        erg = getPrevNumber(point, term)*getNextNumber(point, term);
    }
    else {
        int32_t nn = getNextNumber(point, term);
        if(nn != 0)
            erg = getPrevNumber(point, term)/getNextNumber(point, term);
        else
            printf("Division by zero\n");
    }
    char temp[32];
    itoa(erg, temp);
    size_t strlength = strlen(term)+1;
    char temp2[strlength];
    memcpy(temp2, term, strlength);
    replace(temp2, term, getPrevNumberPos(point, term), getNextNumberPos(point, term) - getPrevNumberPos(point, term)+1, temp);
}
static char* SolveAdd(char* term, size_t point)
{
    int32_t erg = 0;
    if (term[point] == '+') {
        erg = getPrevNumber(point, term)+getNextNumber(point, term);
    }
    else {
        erg = getPrevNumber(point, term)-getNextNumber(point, term);
    }
    char temp[32];
    itoa(erg, temp);
    size_t strlength = strlen(term)+1;
    char temp2[strlength];
    memcpy(temp2, term, strlength);
    replace(temp2, term, getPrevNumberPos(point, term), getNextNumberPos(point, term) - getPrevNumberPos(point, term)+1, temp);
    return(term+point+1);
}

static int32_t CalcTerm(char* term, size_t pos) {
    char temp[32];
    int point = 0;

    while ((point = find_first(term+pos, "(")) != -1) {
        int32_t erg = CalcTerm(term, point+pos+1);
        size_t length = (size_t)(strchr(term+pos, ')') - (term+pos)) - point + 1;
        itoa(erg, temp);
        char temp2[strlen(term+pos)+1];
        strcpy(temp2, term+pos);
        replace(temp2, term+pos, point, length, temp);
        printf("%s\n", term); // Debug output. Shows how the calculator solves terms
    }
    while ((point = find_first(term+pos, "^²")) != -1) {
        SolvePow(term+pos, point);
        printf("%s\n", term); // Debug output. Shows how the calculator solves terms
    }
    while ((point = find_first(term+pos, "*/")) != -1) {
        SolveMul(term+pos, point);
        printf("%s\n", term); // Debug output. Shows how the calculator solves terms
    }
    char* tempterm = term;
    while ((point = find_first(tempterm+pos, "+-")) != -1) {
        tempterm = SolveAdd(tempterm+pos, point);
        printf("%s\n", term); // Debug output. Shows how the calculator solves terms
    }
    return (atoi(term+pos));
}

int main(int argc, char* argv[]) {
    if(argc > 1)
    {
        printf("Result: %i", CalcTerm(argv[1], 0));
        getchar();
        return(CalcTerm(argv[1], 0));
    }

    setScrollField(5, 46);
    printLine("================================================================================", 0, 0x0B);
    printLine("                               Calculator  v0.4.2", 2, 0x0B);
    printLine("--------------------------------------------------------------------------------", 4, 0x0B);
    iSetCursor(0, 6);
    puts("Please type in a term. Valid operators are: +-*/^². Brackets are supported. The Calculator does not support floats at the moment. If you type in \"exit\", the programm will close.\n\n");
    char term[100];
    for (;;) {
        gets_s(term, 100);
        if (strncmp(term, "exit", 4) == 0)
            break;

        int32_t Erg = CalcTerm(term, 0);
        printf("The result of your term is: %i\n\n", Erg);
    }

    return (0);
}

/*
* Copyright (c) 2011-2015 The PrettyOS Project. All rights reserved.
*
* http://www.c-plusplus.de/forum/viewforum-var-f-is-62.html
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
