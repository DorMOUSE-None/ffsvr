#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "ffhttp.h"
#include "ffstr.h"

/**
 * request-line = method SP request-target SP HTTP-version CRLF
 *
 * method:
 *   - GET  Transfer a current representation of the target resource.        
 *   - HEAD Same as GET, but only transfer the status line and header section.   
 *   - POST Perform resource-specific processing on the request payload.     
 *   - PUT  Replace all current representations of the target resource with the request payload.
 *   - DELETE   Remove all current representations of the target resource.
 *   - CONNECT  Establish a tunnel to the server identified by the target resource.
 *   - OPTIONS  Describe the communication options for the target resource.
 *   - TRACE    Perform a message loop-back test along the path to the target resource.
 */
static void parseRequestLine(ffHttpRequest *request, int *p)
{
    ffstr *raw = request->raw;
    // parse method
    request->method = ffCreateString(8); 
    for (;*((raw->buf)+*p) != ' ';(*p)++)
        ffAppendChar(request->method, *(raw->buf+*p));

    // skip space
    (*p)++;

    // parse target
    request->target = ffCreateString(16);
    for (;*((raw->buf)+*p) != ' ';(*p)++)
        ffAppendChar(request->target, *(raw->buf+*p));

    // skip space 
    (*p)++;
    
    // parse version
    request->version = ffCreateString(16);
    for (;*((raw->buf)+*p) != 13;(*p)++)
        ffAppendChar(request->version, *(raw->buf+*p));

    // skip CRLF
    *p += 2;
}

static void parseRequest(ffHttpRequest *request)
{
    int p = 0;
    parseRequestLine(request, &p);
}

static void coreHandle(ffHttpRequest *request, ffHttpResponse *response)
{
    struct stat *fileStat = (struct stat *) malloc(sizeof(struct stat));
    if (stat(request->target->buf, fileStat) == -1)
    {
        // TODO: error
        fprintf(stderr, "target:%s. %s (errno: %d)\n", request->target, strerror(errno), errno);
        return;
    }

    response->body = ffCreateString(fileStat->st_size);
    
    // open target file
    int ffd;
    if ((ffd = open(request->target->buf, O_RDONLY)) == -1)
    {
        // TODO: error
        return;
    }

    // read target file
    if ((response->body->len = read(ffd, response->body->buf, response->body->cap)) == -1)
    {
        // TODO: error
        return;
    }

    // copy request version to response
    response->version = ffCopyString(request->version);
    response->code = ffWrapperString("200");
    response->reason = ffWrapperString("OK");
}

static void encapsulate(ffHttpResponse *response)
{
    ffAppendString(response->raw, response->version);
    ffAppendChar(response->raw, FF_HTTP_SP);
    ffAppendString(response->raw, response->code);
    ffAppendChar(response->raw, FF_HTTP_SP);
    ffAppendString(response->raw, response->reason);
    ffAppendCString(response->raw, FF_HTTP_CRLF);

    ffAppendCString(response->raw, FF_HTTP_CRLF);

    ffAppendString(response->raw, response->body);
    ffAppendCString(response->raw, FF_HTTP_CRLF);
}

ffHttpRequest * ffCreateHttpRequest()
{
    ffHttpRequest *request = (ffHttpRequest *) malloc(sizeof(ffHttpRequest));
    request->raw = ffCreateString(FF_HTTP_REQUEST_RAW_CAP);
    return request;
}

ffHttpResponse * ffCreateHttpResponse()
{
    ffHttpResponse *response = (ffHttpResponse *) malloc(sizeof(ffHttpResponse));
    response->raw = ffCreateString(FF_HTTP_RESPONSE_RAW_CAP);
    return response;
}

/**
 * HTTP Message Format
 * http-message = start-line
 *                * (header-field CRLF)
 *                CRLF
 *                [ message body ]
 * 
 * For request:
 * start-line = request-line
 * request-line = method SP request-target SP HTTP-version CRLF
 *
 * For response:
 * start-line = status-line
 * status-line = HTTP-version SP status-code SP reason-phrase CRLF
 */
int ffHttpHandle(ffHttpRequest *request, ffHttpResponse *response)
{
    // parse request raw message
    parseRequest(request);
    // core handle to read request and make response
    coreHandle(request, response);
    // encapsulate response
    encapsulate(response);
}

void ffReleaseRequest(ffHttpRequest *request)
{
    if (request->raw != NULL)
        ffReleaseString(request->raw);
    if (request->method != NULL)
        ffReleaseString(request->method);
    if (request->target != NULL)
        ffReleaseString(request->target);
    if (request->version != NULL)
        ffReleaseString(request->version);
    free(request);
}

void ffReleaseResponse(ffHttpResponse *response)
{
    if (response->raw != NULL)
        ffReleaseString(response->raw);
    free(response);
}
