#ifndef FF_CLT_H
#define FF_CLT_H

#include "ffstr.h"
#include "ffhttp.h"

typedef struct ffcltClient {
    char *ip;           // Client IP
    uint16_t port;      // Client port
    int fd;             // socket fd

    ffHttpRequest *request;     // request
    ffHttpResponse *response;   // response
} ffcltClient;

ffcltClient * ffCreateClient();
void ffReleaseClient(ffcltClient *clt);

#endif /* FF_CLT_H */
