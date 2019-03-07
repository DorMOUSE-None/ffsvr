#ifndef FF_STR_H
#define FF_STR_H

struct ffStr {
    char *buf;      // str buffer
    int len;        // str length
    int cap;        // capability
};

typedef struct ffStr ffstr;

ffstr * ffCreateNewString(int initCapability);

#endif /* FF_STR_H */
