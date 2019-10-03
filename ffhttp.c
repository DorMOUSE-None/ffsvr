#ifdef FF_HTTP_ON

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "ffhttp.h"
#include "ffstr.h"

static void ffhttpSetError(char *err, const char *fmt, ...)
{
    va_list val;
    
    if (err == NULL)
        return;
    va_start(val, fmt);
    vsnprintf(err, FF_HTTP_ERR_LEN, fmt, val);
    va_end(val);
}

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
    for (;*((raw->buf)+*p) != ' ';(*p)++)
    {
        if (*(raw->buf+*p) == '?')
        {
            for (;*(raw->buf+*p) != ' ';(*p)++);
            (*p)--;
            continue;
        }
        ffAppendChar(request->target, *(raw->buf+*p));
    }

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

static int coreHandle(char *err, ffHttpRequest *request, ffHttpResponse *response)
{
    struct stat *fileStat = (struct stat *) malloc(sizeof(struct stat));
    fprintf(stderr, "read file %s\n", request->target->buf);
    if (stat(request->target->buf, fileStat) == -1)
    {
        ffhttpSetError("stat file %s failed. %s (errno: %d)\n", request->target->buf, strerror(errno), errno); 
        return FF_HTTP_ERR;
    }

    response->body = ffCreateString(fileStat->st_size);
    free(fileStat);
    
    // open target file
    int ffd;
    if ((ffd = open(request->target->buf, O_RDONLY)) == -1)
    {
        ffhttpSetError("open file %s failed. %s (errno: %d)\n", request->target->buf, strerror(errno), errno); 
        return FF_HTTP_ERR;
    }

    // read target file
    if ((response->body->len = read(ffd, response->body->buf, response->body->cap)) == -1)
    {
        ffhttpSetError("read file %s failed. %s (errno: %d)\n", request->target->buf, strerror(errno), errno); 
        return FF_HTTP_ERR;
    }

    // copy request version to response
    response->version = ffCopyString(request->version);
    response->code = ffWrapperString("200");
    response->reason = ffWrapperString("OK");

    // generate header 
    response->contentLength = ffCreateString(8);
    sprintf(response->contentLength->buf, "%d", response->body->len);
    response->contentLength->len = strlen(response->contentLength->buf);

    response->server = ffWrapperString(FF_HTTP_SERVER);

    return FF_HTTP_OK;
}

static void encapsulate(ffHttpResponse *response)
{
    ffAppendString(response->raw, response->version);
    ffAppendChar(response->raw, FF_HTTP_SP);
    ffAppendString(response->raw, response->code);
    ffAppendChar(response->raw, FF_HTTP_SP);
    ffAppendString(response->raw, response->reason);
    ffAppendCString(response->raw, FF_HTTP_CRLF);

    ffAppendCString(response->raw, "server: ");
    ffAppendString(response->raw, response->server);
    ffAppendCString(response->raw, FF_HTTP_CRLF);
    ffAppendCString(response->raw, "content-length: ");
    ffAppendString(response->raw, response->contentLength);
    ffAppendCString(response->raw, FF_HTTP_CRLF);

    ffAppendCString(response->raw, FF_HTTP_CRLF);

    ffAppendString(response->raw, response->body);
    ffAppendCString(response->raw, FF_HTTP_CRLF);
}

void ffHttpSetWorkdir(char *dir)
{
    if (workdir != NULL)
        ffReleaseString(workdir);
    workdir = ffWrapperString(dir);
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
int ffHttpHandle(char *err, ffHttpRequest *request, ffHttpResponse *response)
{
    // init prefix of target
    request->target = ffCopyString(workdir);
    // parse request raw message
    parseRequest(request);
    // core handle to read request and make response
    if (coreHandle(err, request, response) == FF_HTTP_ERR)
        return FF_HTTP_ERR;
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

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "ffhttp.h"
#include "ffstr.h"

static void ffhttpSetError(char *err, const char *fmt, ...)
{
    va_list val;
    
    if (err == NULL)
        return;
    va_start(val, fmt);
    vsnprintf(err, FF_HTTP_ERR_LEN, fmt, val);
    va_end(val);
}

static unsigned int stoi(char *str) 
{
    unsigned int value = 0;
    for (str++;(*str)!=' ';str++) {
        value *= 10;
        value += (*str - '0');
    }
    return value;
}

static unsigned int primeCnt(unsigned int value) 
{
    // fatal function TODO:
    return value;
}

static int length(unsigned int value) 
{
    int cnt = 0;
    for (;value != 0;value/=10, cnt++);
    return cnt;
}

void ffHttpSetWorkdir(char *dir)
{
    if (workdir != NULL)
        ffReleaseString(workdir);
    workdir = ffWrapperString(dir);
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
int ffHttpHandle(char *err, ffHttpRequest *request, ffHttpResponse *response)
{
    /**
     * locate PATH pointer
     */
    int p = 0;
    ffstr *raw = request->raw;
    // skip method
    for (;*(raw->buf+p) != ' ';p++);
    // skip space
    p++;

    unsigned int value = stoi(request->raw->buf+p);
    unsigned int pi_n = primeCnt(value);

    sprintf(response->raw->buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%u=%u\r\n", length(value) + length(pi_n) + 1, value, pi_n);
    response->raw->len = strlen(response->raw->buf);
    return FF_HTTP_OK;
}

void ffReleaseRequest(ffHttpRequest *request)
{
    if (request->raw != NULL)
        ffReleaseString(request->raw);
    free(request);
}

void ffReleaseResponse(ffHttpResponse *response)
{
    if (response->raw != NULL)
        ffReleaseString(response->raw);
    free(response);
}
#endif
