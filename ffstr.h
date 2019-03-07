#ifndef FF_STR_H
#define FF_STR_H

struct ffStr {
    char *buf;      // str buffer
    int len;        // str length
    int cap;        // capability
};

typedef struct ffStr ffstr;

ffstr * ffCreateNewString(int initCapability);
void ffAppendChar(ffstr *str, char c);
void ffReleaseCurrentString(ffstr *str);

#endif /* FF_STR_H */
