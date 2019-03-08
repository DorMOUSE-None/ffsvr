#ifndef FF_HTTP_H
#define FF_HTTP_H

#include "ffstr.h"

#define FF_HTTP_METHOD_GET "GET"
#define FF_HTTP_METHOD_POST "POST"

#define FF_HTTP_SP " "
#define FF_HTTP_CRLF "\r\n"

#define FF_HTTP_REQUEST_RAW_CAP 1024
#define FF_HTTP_RESPONSE_RAW_CAP 65536

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
    ffstr *body;        // message-body
} ffHttpResponse;

ffHttpRequest * ffCreateHttpRequest();
ffHttpResponse * ffCreateHttpResponse();
int ffHttpHandle(ffHttpRequest *request, ffHttpResponse *response);
void ffReleaseRequest(ffHttpRequest *request);
void ffReleaseResponse(ffHttpResponse *response);

#endif /* FF_HTTP_H */
