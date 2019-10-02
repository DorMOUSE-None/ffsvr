#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "ffclt.h"
#include "fftcp.h"
#include "ffhttp.h"
#include "ffstr.h"

#define FFSVR_VERSION "0.1"
#define FFSVR_DEFAULT_PORT "3960"
#define FFSVR_EVENTS_BLOCKS 100
#define FFSVR_MAX_FDS 65536

/* Log Levels */
#define FFSVR_DEBUG 0
#define FFSVR_INFO 1
#define FFSVR_WARN 2
#define FFSVR_ERROR 3

struct ffsvrServer {
    char *port;             // 监听端口
    char *bindaddr;         // 绑定地址 
    int fd;                 // socket 文件描述符
    int epfd;               // epoll file descriptor

    char *err;              // error message
    char *logfile;          // 日志文件
    int verbosity;          // verbose

    char *workdir;          // 工作目录
};

struct ffsvrFd {
    void *ptr;
    void *extra;
};

struct ffsvrServer server;
struct ffsvrFd fd_data[65536];

static void ffsvrLog(int logLevel, const char *fmt, ...) {
    va_list val;
    FILE *fp;

    if (logLevel < server.verbosity)
        return;

    fp = (server.logfile == NULL) ? stdout : fopen(server.logfile, "a");
    if (fp == NULL) 
        return;

    // 时间格式化 & 打印日志消息
    char *c = ".-#@";
    time_t now;
    char buf[64];

    now = time(NULL);
    strftime(buf,64,"%d %b %H:%M:%S",gmtime(&now));
    fprintf(fp,"%s %c ",buf,c[logLevel]);
    va_start(val, fmt);
    vfprintf(fp, fmt, val);
    va_end(val);
    fprintf(fp, "\n");
    fflush(fp);

    if (server.logfile)
        pclose(fp);
}

static void banner() {
    ffsvrLog(FFSVR_DEBUG, "%s", "\n\
 _____ _____ ______     ______ \n\
|  ___|  ___/ ___\\ \\   / /  _ \\ \n\
| |_  | |_  \\___ \\\\ \\ / /| |_) | \n\
|  _| |  _|  ___) |\\ V / |  _ < \n\
|_|   |_|   |____/  \\_/  |_| \\_\\");
}


void http_handle(int acptfd, struct sockaddr_in *addr, socklen_t addrlen);

static void usage()
{
    fprintf(stderr, "Usage: ffsvr [option]\n");
    fprintf(stderr, "\t-d workdir\n");
    fprintf(stderr, "\t-l ip\n");
    fprintf(stderr, "\t-p port\n");
    fprintf(stderr, "\t-h\n");
    exit(0);
}

static void initServer() {
    server.port = FFSVR_DEFAULT_PORT;
    server.bindaddr = NULL;

    server.err = (char *) malloc(FF_TCP_ERR_LEN);
    server.logfile = NULL;
    server.verbosity = FFSVR_DEBUG;

    server.workdir = NULL;
}

static void configServer(int argc, char **argv) {
    int opt;
    while ((opt = getopt(argc, argv, "d:l:p:h")) != -1) 
    {
        switch (opt)
        {
            case 'd':
                server.workdir = strdup(optarg);
                ffHttpSetWorkdir(server.workdir);
                break;
            case 'l':
                server.bindaddr = strdup(optarg);
                break;
            case 'p':
                server.port = strdup(optarg);
                break;
            case 'h':
            default:
                usage();
                return;
        }
    }
}

void (*ep_handler)(int epfd, int fd, void *extraData);
void sendFunc(int epfd, int fd, void *extraData) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    struct ffcltClient *clt = (struct ffcltClient *) extraData;

    // handle http request to response
    if (ffHttpHandle(server.err, clt->request, clt->response) == FF_HTTP_ERR) {
        ffsvrLog(FFSVR_ERROR, server.err);
        // TODO: 404
        return;
    }

    ffstr *sendStr = clt->response->raw;
    int retval = send(fd, sendStr->buf, sendStr->len, 0);
    if (retval == -1)
    {
        ffsvrLog(FFSVR_ERROR, "send to Client %s:%d Failed", clt->ip, clt->port);
        return;
    }
    ffsvrLog(FFSVR_DEBUG, "send message to Client %s:%d.", clt->ip, clt->port);
}

void recvFunc(int epfd, int fd, void *extraData) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    struct ffcltClient *clt = (struct ffcltClient *) extraData;

    ffstr *recvStr = clt->request->raw;
    recvStr->len = recv(fd, recvStr->buf, recvStr->cap, 0);
    if (recvStr->len == -1)
    {
        ffsvrLog(FFSVR_ERROR, "recv from Client %s:%d Failed", clt->ip, clt->port);
        return; 
    } 
    else if (recvStr->len == 0)
    {
        ffsvrLog(FFSVR_INFO, "disconnect with Client %s:%d.", clt->ip, clt->port);
        ffReleaseClient(clt);
        return;
    }

    recvStr->buf[recvStr->len] = '\0';
    ffsvrLog(FFSVR_DEBUG, "recv message from Client %s:%d. message: %s", clt->ip, clt->port, recvStr->buf);
    
    
    struct epoll_event ep_event;
    ep_event.events = EPOLLOUT;
    ep_event.data.u32 = clt->fd;
    fd_data[clt->fd].ptr = &sendFunc;
    fd_data[clt->fd].extra = extraData;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ep_event) == -1)
        ffsvrLog(FFSVR_WARN, "create New File Event Failed!");
}

void acceptFunc(int epfd, int fd, void *extraData) {
    ffcltClient *clt = ffCreateClient();
    if ((clt->fd = fftcpAccept(server.err, fd, clt->ip, &clt->port)) == FF_TCP_ERR) {
        ffsvrLog(FFSVR_ERROR, "%s", server.err);
        free(server.err);
        return; 
    }
    ffsvrLog(FFSVR_INFO, "Accept %s:%d", clt->ip, clt->port);

    int retval;
    struct epoll_event ep_event;
    ep_event.events = EPOLLIN;
    ep_event.data.u32 = clt->fd;
    fd_data[clt->fd].ptr = &recvFunc;
    fd_data[clt->fd].extra = clt;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, clt->fd, &ep_event) == -1) {
        ffsvrLog(FFSVR_WARN, "Create New File Event Failed! %s (errno: %d)\n", strerror(errno), errno);
    }
}

int main(int argc, char **argv)
{
    // init default params
    initServer();
    banner();
    ffsvrLog(FFSVR_DEBUG, "%s", "Hello FFSVR!");

    // read command params
    configServer(argc, argv);

    if ((server.fd = fftcpServer(server.err, server.port, server.bindaddr)) == FF_TCP_ERR) { 
        fprintf(stderr, "%s\n", server.err);
        free(server.err);
        return 0;
    }

    // create eventpoll
    if ((server.epfd = epoll_create(1)) == -1) {
        ffsvrLog(FFSVR_ERROR, "%s (errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    // register first event
    struct epoll_event ep_event;
    ep_event.events = EPOLLIN;
    ep_event.data.u32 = server.fd;
    fd_data[server.fd].ptr = &acceptFunc;
    fd_data[server.fd].extra = NULL;
    epoll_ctl(server.epfd, EPOLL_CTL_ADD, server.fd, &ep_event);

    // start event loop
    struct epoll_event ep_events[FFSVR_EVENTS_BLOCKS];
    for (;;) {
        int num = epoll_wait(server.epfd, ep_events, FFSVR_EVENTS_BLOCKS, 100);
        if (num == -1) {
            ffsvrLog(FFSVR_ERROR, "%s (errno: %d)\n", strerror(errno), errno);
            continue;
        }
        for (int i=0, fd;i<num;i++) {
            fd = ep_events[i].data.u32;
            ep_handler = fd_data[fd].ptr;
            (*ep_handler)(server.epfd, fd, fd_data[fd].extra);
        }
    }
}
