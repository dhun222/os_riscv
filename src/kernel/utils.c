#include "types.h"
#include "utils.h"

void memcpy(char *src, char *dest, size_t size)
{
    while (size--) {
        dest[size] = src[size];
    }
    return;
}