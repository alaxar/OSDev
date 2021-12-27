#ifndef SIGNAL_H
#define SIGNAL_H

enum {
    SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM, _SIGNUM
};

extern void (*SIG_DFL)(int);
extern void (*SIG_IGN)(int);

typedef int sig_atomic_t;


#ifdef _cplusplus
extern "C" {
#endif

void (*signal(int sig, void (*func)(int)))(int);
int raise(int sig);

#ifdef _cplusplus
}
#endif

#endif
