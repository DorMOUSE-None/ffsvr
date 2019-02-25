#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "fftcp.h"

#define FFSVR_VERSION "0.1"
#define FFSVR_DEFAULT_PORT "3960"

/* Log Levels */
#define FFSVR_DEBUG 0
#define FFSVR_INFO 1
#define FFSVR_WARN 2
#define FFSVR_ERROR 3

#define RECV_BUF_LEN 1024
#define SEND_BUF_LEN 1024

struct ffsvrServer {
    char *port;             // 监听端口
    char *bindaddr;         // 绑定地址 
    int fd;                 // socket 文件描述符

    char *logfile;          // 日志文件
    int verbosity;          // verbose
};

static struct ffsvrServer server;

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
        close(fp);
}

static void banner() {
    ffsvrLog(FFSVR_DEBUG, "%s", "\n\
 _____ _____ ______     ______ \n\
|  ___|  ___/ ___\\ \\   / /  _ \\ \n\
| |_  | |_  \\___ \\\\ \\ / /| |_) | \n\
|  _| |  _|  ___) |\\ V / |  _ < \n\
|_|   |_|   |____/  \\_/  |_| \\_\\");
}

char *recv_buf, *send_buf;
int recv_buf_len, send_buf_len;

void http_handle(int acptfd, struct sockaddr_in *addr, socklen_t addrlen);

void usage()
{
    fprintf(stderr, "Usage: ffsvr [option]\n");
    fprintf(stderr, "\t-l ip\n");
    fprintf(stderr, "\t-p port\n");
    fprintf(stderr, "\t-h\n");
}

void initServerConfig() {
    server.port = FFSVR_DEFAULT_PORT;
    server.bindaddr = NULL;

    server.logfile = NULL;
    server.verbosity = FFSVR_DEBUG;
}

int main(int argc, char **argv)
{
    initServerConfig();
    banner();
    ffsvrLog(FFSVR_DEBUG, "%s", "Hello FFSVR!");
    const char *ip_bind = NULL;
    const char *port_listen = "3960";
    int sockfd;
    int backlog = 5;
    int return_code;
    struct addrinfo hint, *rst;

    recv_buf = (char *) malloc(RECV_BUF_LEN);
    recv_buf_len = RECV_BUF_LEN;
    send_buf = (char *) malloc(SEND_BUF_LEN);
    send_buf_len = SEND_BUF_LEN;
    
    int opt;
    while ((opt = getopt(argc, argv, "l:p:h")) != -1) 
    {
        switch (opt)
        {
            case 'l':
                ip_bind = strdup(optarg);
                break;
            case 'p':
                port_listen = strdup(optarg);
                break;
            case 'h':
            default:
                usage();
                return 0;
        }
    }

    char *err = (char *) malloc(FF_TCP_ERR_LEN);

    if ((sockfd = fftcpServer(&err, port_listen, ip_bind)) == FF_TCP_ERR) { 
        fprintf(stderr, "%s\n", err);
        free(err);
        return 0;
    }

    while (1)
    {
        char *ip = (char *) malloc(INET_ADDRSTRLEN);
        uint16_t port;
        int acptfd;
        if ((acptfd = fftcpAccept(err, sockfd, ip, &port)) == FF_TCP_ERR) {
            fprintf(stderr, "%s\n", err);
            free(err);
            return 0;
        }
        printf("Accepted %s:%d\n", ip, port);
        int recv_len = recv(acptfd, recv_buf, recv_buf_len, 0);
        if (recv_len == -1)
        {
            fprintf(stderr, "[ERROR] recv msg failed. %s (errno=%d)\n", strerror(errno), errno);
            return 0;
        }
        recv_buf[recv_len] = '\0';
        printf("[DEBUG] recv msg: %s\n", recv_buf);
    
        char *http_response = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\n1234567890";
        int resp_len = strlen(http_response);
        if (-1 == send(acptfd, http_response, resp_len, 0))
        {
            fprintf(stderr, "[ERROR] send msg failed. %s (errno=%d)\n", strerror(errno), errno);
            return 0;
        }
           
        /*http_handle(acptfd, (struct sockaddr_in *) &addr, addrlen);*/
    }
}

void http_handle(int acptfd, struct sockaddr_in *addr, socklen_t addrlen)
{
    char ip[14];
    if (addr->sin_family == AF_INET6)
        printf("Oh, My God!\n");
    inet_ntop(addr->sin_family, &(addr->sin_addr), ip, sizeof ip);
    printf("%s\n", ip);

    int recv_len = recv(acptfd, recv_buf, recv_buf_len, 0);
    if (recv_len == -1)
    {
        fprintf(stderr, "[ERROR] recv msg failed. %s (errno=%d)\n", strerror(errno), errno);
        return;
    }
    recv_buf[recv_len] = '\0';
    printf("[DEBUG] recv msg: %s\n", recv_buf);

    char *http_response = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\n1234567890";
    int resp_len = strlen(http_response);
    if (-1 == send(acptfd, http_response, resp_len, 0))
    {
        fprintf(stderr, "[ERROR] send msg failed. %s (errno=%d)\n", strerror(errno), errno);
        return;
    }
}
