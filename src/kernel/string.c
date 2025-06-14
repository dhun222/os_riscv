#include "string.h"

// Unsafe string copy funciton. Ends when meets NULL in src
void strcpy(char *dst, char *src)
{
    int i = 0;
    while(src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
}