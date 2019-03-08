#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "ffclt.h"
#include "ffstr.h"
#include "ffhttp.h"

ffcltClient * ffCreateClient() 
{
    ffcltClient *clt = (ffcltClient *) malloc(sizeof(ffcltClient));

    clt->ip = (char *) malloc(INET_ADDRSTRLEN);

    clt->request = ffCreateHttpRequest();
    clt->response = ffCreateHttpResponse();

    return clt;
}

void ffReleaseClient(struct ffcltClient *clt)
{
    // release request & response
    ffReleaseRequest(clt->request);
    ffReleaseResponse(clt->response);
    free(clt->ip);
    free(clt);
}
