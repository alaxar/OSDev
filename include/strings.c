#include "strings.h"

int string_compare(unsigned char *str1, unsigned char *str2) {
    int is_equal = 0;
    unsigned char *p1 = (unsigned char *)str1;
    unsigned char *p2 = (unsigned char *)str2;    


    while(*p1 != '\0' || *p2 != '\0') {
        if(*p1 == *p2)
            is_equal++;
        else
            return 0;

        p1++;
        p2++;
    }

    if(is_equal > 0)
        return is_equal;
    else 
        return 0;           // 0 for not equal

}


int string_length(unsigned char *str) {
    int count = 0;
    while(*str != '\0') {
        count++;
        str++;
    }
    return count;
}