#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#include "ffevent.h"

ffEventLoop * ffeventCreateEventLoop()
{
    ffEventLoop *eventLoop;

    eventLoop = (ffEventLoop *) malloc(sizeof(ffEventLoop));
    if (eventLoop == NULL)
        return NULL;

    eventLoop->maxfd = 0;
    eventLoop->fileEvent = NULL;
    eventLoop->stop = 0;
    return eventLoop;
}

int ffeventCreateFileEvent(ffEventLoop *eventLoop, int fd, int mask, ffFileFunc *func, void *extraData)
{
    ffFileEvent *fileEvent;

    fileEvent = (ffFileEvent *) malloc(sizeof(ffFileEvent)); 
    if (fileEvent == NULL)
        return FF_EVENT_ERR;

    fileEvent->fd = fd;
    fileEvent->mask = mask;
    fileEvent->func = func;
    fileEvent->extraData = extraData;
    fileEvent->next = eventLoop->fileEvent;
    eventLoop->fileEvent = fileEvent;
    if (eventLoop->maxfd < fd)
        eventLoop->maxfd = fd;

    return FF_EVENT_OK;
}

int ffeventDeleteFileEvent(ffEventLoop *eventLoop, int fd, int mask)
{
    ffFileEvent *fileEvent, *prev = NULL;

    fileEvent = eventLoop->fileEvent;
    while (fileEvent != NULL) {
        if (fileEvent->fd == fd && fileEvent->mask == mask) {
            if (prev == NULL) 
                eventLoop->fileEvent = fileEvent->next;
            else 
                prev->next = fileEvent->next;
            free (fileEvent);
            return FF_EVENT_OK;
        }
        prev = fileEvent;
        fileEvent = fileEvent->next;
    }
    return FF_EVENT_ERR;
}

void ffeventProcess(ffEventLoop *eventLoop) {
    fd_set rfds, wfds, efds;
    int retval, fd, mask;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    ffFileEvent *fileEvent = eventLoop->fileEvent;
    while (fileEvent != NULL)
    {
        if (fileEvent->mask & FF_EVENT_MASKREAD)
            FD_SET(fileEvent->fd, &rfds);
        if (fileEvent->mask & FF_EVENT_MASKWRITE)
            FD_SET(fileEvent->fd, &wfds);
        if (fileEvent->mask & FF_EVENT_MASKERR)
            FD_SET(fileEvent->fd, &efds);
        fileEvent = fileEvent->next;
    }

    retval = select(eventLoop->maxfd + 1, &rfds, &wfds, &efds, NULL);
    if (retval == -1)
        // TODO:
        return;
    else {
        fileEvent = eventLoop->fileEvent;
        while (fileEvent != NULL)
        {
            fd = fileEvent->fd;
            mask = 0;
            if (fileEvent->mask & FF_EVENT_MASKREAD && FD_ISSET(fd, &rfds))
                mask |= FF_EVENT_MASKREAD;
            if (fileEvent->mask & FF_EVENT_MASKWRITE && FD_ISSET(fd, &wfds))
                mask |= FF_EVENT_MASKWRITE;
            if (fileEvent->mask & FF_EVENT_MASKERR && FD_ISSET(fd, &efds))
                mask |= FF_EVENT_MASKERR;
            // callback func
            if (mask != 0)
                fileEvent->func(eventLoop, fd, mask, fileEvent->extraData);
            // next event
            fileEvent = fileEvent->next;
        }
    }
}

void ffeventDispatch(ffEventLoop *eventLoop)
{
    eventLoop->stop = 0;
    while (!eventLoop->stop)
        ffeventProcess(eventLoop);
}
