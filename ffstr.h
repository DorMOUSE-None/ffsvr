#ifndef FF_STR_H
#define FF_STR_H

typedef struct ffStr {
    char *buf;      // str buffer
    int len;        // str length
    int cap;        // capability
} ffstr;

ffstr * ffCreateString(int initCapability);
ffstr * ffWrapperString(char *src);
ffstr * ffCopyString(ffstr *str);
void ffAppendChar(ffstr *str, char c);
void ffAppendCString(ffstr *dest, char *src);
void ffAppendString(ffstr *dest, ffstr *src);
void ffReleaseString(ffstr *str);

#endif /* FF_STR_H */
