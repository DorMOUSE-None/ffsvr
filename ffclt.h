#ifndef FF_CLT_H
#define FF_CLT_H

#define FFCLT_RECV_BUF_CAP 1024
#define FFCLT_SEND_BUF_CAP 1024

struct ffcltClient {
    char *ip;           // Client IP
    uint16_t port;      // Client port
    int fd;             // socket fd

    char *recvBuf;      // 接收缓冲区
    int recvBufLen;     // 接收数据长度
    int recvBufCap;     // 总长度
    
    char *sendBuf;      // 发送缓冲区
    int sendBufLen;     // 发送数据长度
    int sendBufCap;     // 总长度
};

struct ffcltClient * ffcltInitClient();

#endif /* FF_CLT_H */
