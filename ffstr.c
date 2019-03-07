#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ffstr.h"

ffstr * ffCreateNewString(int initCapability)
{
    // ffstr's capability should be power of 2
    int capability = 1;
    while (initCapability > capability)
        capability <<= 1;

    struct ffStr *str = (struct ffStr *) malloc(sizeof(struct ffStr));
    str->buf = (char *) malloc(capability);
    str->len = 0;
    // reserve 1 byte for '\0'
    str->cap = capability - 1;
    return str;
}

void ffAppendChar(ffstr *str, char c)
{
    // not have enough space
    if (str->len == str->cap)
    {
        int capability = str->cap + 1;
        str->buf = realloc(str->buf, capability <<= 1);
        str->cap = capability - 1; 
    }
    str->buf[str->len++] = c;
    str->buf[str->len] = '\0';
}

void ffReleaseCurrentString(ffstr *str)
{
    free(str->buf);
    free(str);
}
