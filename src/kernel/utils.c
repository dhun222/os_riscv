#include "types.h"
#include "utils.h"

void memcpy(char *src, char *dest, uint64 size)
{
    while (size--) {
        dest[size] = src[size];
    }
    return;
}