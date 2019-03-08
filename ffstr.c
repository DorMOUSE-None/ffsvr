#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ffstr.h"

ffstr * ffCreateString(int initCapability)
{
    // ffstr's capability should be power of 2
    int capability = 1;
    while (initCapability > capability)
        capability <<= 1;

    ffstr *str = (ffstr *) malloc(sizeof(ffstr));
    str->buf = (char *) malloc(capability);
    str->len = 0;
    // reserve 1 byte for '\0'
    str->cap = capability - 1;
    return str;
}

ffstr * ffWrapperString(char *src)
{
    ffstr *dest = (ffstr *) malloc(sizeof(ffstr));
    int length = strlen(src);
    int capability = 1;
    while (length > capability)
        capability <<= 1;
    dest->buf = (char *) malloc(capability);
    dest->len = length;
    dest->cap = capability - 1;
    strcpy(dest->buf, src);
    return dest;
}

ffstr * ffCopyString(ffstr *src)
{
    ffstr *dest = (ffstr *) malloc(sizeof(ffstr));
    dest->buf = (char *) malloc(src->cap + 1);
    strcpy(dest->buf, src->buf);
    dest->len = src->len;
    dest->cap = src->cap;
    return dest;
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

void ffAppendCString(ffstr *dest, char *src)
{
    int srcLen = strlen(src);
    // not have enough space
    if (dest->len + srcLen > dest->cap)
    {
        int capability = dest->cap + 1;
        while (capability <= dest->len + srcLen)
            capability <<= 1;
        dest->buf = realloc(dest->buf, capability);
        dest->cap = capability - 1;
    }
    strcpy(dest->buf+dest->len, src);
    dest->len += srcLen;
}

void ffAppendString(ffstr *dest, ffstr *src)
{
    // not have enough space
    if (dest->len + src->len > dest->cap)
    {
        int capability = dest->cap + 1;
        while (capability <= dest->len + src->len)
            capability <<= 1;
        dest->buf = realloc(dest->buf, capability);
        dest->cap = capability - 1;
    }
    strcpy(dest->buf+dest->len, src->buf);
    dest->len += src->len;
}

void ffReleaseString(ffstr *str)
{
    free(str->buf);
    free(str);
}
