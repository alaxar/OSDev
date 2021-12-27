#ifndef SETJMP_H
#define SETJMP_H


struct jmp_buf {
    unsigned long esp;
    unsigned long ebp;
    unsigned long oldebp;
    unsigned long jmpAdress;
    unsigned long returnAdress;
};
typedef struct jmp_buf jmp_buf[1];


#ifdef _cplusplus
extern "C" {
#endif

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int retVal);

#ifdef _cplusplus
}
#endif

#endif
