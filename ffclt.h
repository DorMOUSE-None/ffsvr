#ifndef FF_CLT_H
#define FF_CLT_H

#include "ffstr.h"

#define FFCLT_RECV_BUF_CAP 1024
#define FFCLT_SEND_BUF_CAP 1024

struct ffcltClient {
    char *ip;           // Client IP
    uint16_t port;      // Client port
    int fd;             // socket fd

    ffstr *recv;        // 接收缓冲
    ffstr *send;        // 发送缓冲
};

struct ffcltClient * ffcltInitClient();
void ffcltCloseClient(struct ffcltClient *clt);

#endif /* FF_CLT_H */
