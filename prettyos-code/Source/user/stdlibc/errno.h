#ifndef ERRNO_H
#define ERRNO_H

#define EDOM 1
#define ERANGE 2
#define EILSEQ 3

int* _errno(void);
#define errno (*_errno())

#endif
