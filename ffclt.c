#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "ffclt.h"

struct ffcltClient * ffcltInitClient() 
{
    struct ffcltClient *clt = (struct ffcltClient *) malloc(sizeof(struct ffcltClient));

    clt->ip = (char *) malloc(INET_ADDRSTRLEN);

    clt->recvBuf = (char *) malloc(FFCLT_RECV_BUF_CAP);
    clt->recvBufLen = 0;
    // reserve 1 byte for '\0'
    clt->recvBufCap = FFCLT_RECV_BUF_CAP - 1;

    clt->sendBuf = (char *) malloc(FFCLT_SEND_BUF_CAP);
    clt->sendBufLen = 0;
    // reserve 1 byte for '\0'
    clt->sendBufCap = FFCLT_SEND_BUF_CAP - 1;

    return clt;
}
