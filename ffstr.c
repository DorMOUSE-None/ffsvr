#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ffstr.h"

ffstr * ffCreateNewString(int initCapability)
{
    struct ffStr *str = (struct ffStr *) malloc(sizeof(struct ffStr));
    str->buf = (char *) malloc(initCapability);
    str->len = 0;
    // reserve 1 byte for '\0'
    str->cap = initCapability - 1;
    return str;
}
