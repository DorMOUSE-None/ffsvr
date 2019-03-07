#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "ffclt.h"
#include "ffstr.h"

struct ffcltClient * ffcltInitClient() 
{
    struct ffcltClient *clt = (struct ffcltClient *) malloc(sizeof(struct ffcltClient));

    clt->ip = (char *) malloc(INET_ADDRSTRLEN);

    clt->recv = ffCreateNewString(FFCLT_RECV_BUF_CAP);
    clt->send = ffCreateNewString(FFCLT_SEND_BUF_CAP);

    return clt;
}

void ffcltCloseClient(struct ffcltClient *clt)
{
    // release strings
    ffReleaseCurrentString(clt->recv);
    ffReleaseCurrentString(clt->send);

    free(clt->ip);
    free(clt);
}
