#ifndef FF_HTTP_H
#define FF_HTTP_H

#include "ffstr.h"

#define FF_HTTP_ERR -1
#define FF_HTTP_OK 0
#define FF_HTTP_ERR_LEN 128

#define FF_HTTP_METHOD_GET "GET"
#define FF_HTTP_METHOD_POST "POST"

#define FF_HTTP_SP 32
#define FF_HTTP_CRLF "\r\n"

#define FF_HTTP_SERVER "ffsvr"

#define FF_HTTP_REQUEST_RAW_CAP 1024
#define FF_HTTP_RESPONSE_RAW_CAP 65536

static ffstr *workdir;

typedef struct ffHttpRequest {
    ffstr *raw;
    ffstr *method;
    ffstr *target;
    ffstr *version;
} ffHttpRequest;

typedef struct ffHttpResponse {
    ffstr *raw;
    ffstr *version;     // HTTP-version
    ffstr *code;        // status-code
    ffstr *reason;      // reason-phrase

    ffstr *server;
    ffstr *contentLength; 

    ffstr *body;        // message-body
} ffHttpResponse;

void ffHttpSetWorkdir(char *workdir);
ffHttpRequest * ffCreateHttpRequest();
ffHttpResponse * ffCreateHttpResponse();
int ffHttpHandle(char *err, ffHttpRequest *request, ffHttpResponse *response);
void ffReleaseRequest(ffHttpRequest *request);
void ffReleaseResponse(ffHttpResponse *response);

#endif /* FF_HTTP_H */
