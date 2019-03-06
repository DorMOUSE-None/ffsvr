#ifndef FF_EVENT_H
#define FF_EVENT_H

#define FF_EVENT_ERR -1
#define FF_EVENT_OK 0

#define FF_EVENT_MASKREAD 0b001
#define FF_EVENT_MASKWRITE 0b010
#define FF_EVENT_MASKERR 0b100

struct ffEventLoop;
struct ffFileEvent;

typedef void ffFileFunc(struct ffEventLoop *eventLoop, int fd, int mask, void *extraData);

typedef struct ffFileEvent {
    int fd;
    int mask;
    ffFileFunc *func;
    void *extraData;
    struct FileEvent *next;
} ffFileEvent;

typedef struct ffEventLoop {
    int maxfd;
    ffFileEvent *fileEvent;
    int stop;
} ffEventLoop;

ffEventLoop * ffeventCreateEventLoop();
int ffeventCreateFileEvent(ffEventLoop *eventLoop, int fd, int mask, ffFileFunc *func, void *extra);
int ffeventDeleteFileEvent(ffEventLoop *eventLoop, int fd, int mask);
void ffeventDispatch(ffEventLoop *eventLoop);

#endif /* FF_EVENT_H */
