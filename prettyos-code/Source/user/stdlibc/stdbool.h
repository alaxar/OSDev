#ifndef STDBOOL_H
#define STDBOOL_H

#ifndef __bool_true_false_are_defined
  #ifndef _cplusplus
    typedef _Bool bool;
    #define true  1
    #define false 0
  #endif
  #define __bool_true_false_are_defined 1
#endif

#endif
