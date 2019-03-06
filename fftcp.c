#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fftcp.h"

static void fftcpSetError(char *err, const char *fmt, ...) {
    va_list val;

    if (!err) return;
    va_start(val, fmt);
    vsnprintf(err, FF_TCP_ERR_LEN, fmt, val);
    va_end(val);
}

int fftcpServer(char *err, char *port, char *bindaddr) {
    
    int sockfd;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fftcpSetError(err, "socket: %s(errno: %d)\n", strerror(errno), errno);
        return FF_TCP_ERR;
    }

    struct addrinfo hint, *res;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    if (getaddrinfo(bindaddr, port, &hint, &res) != 0) {
        fftcpSetError(err, "getaddrinfo: %s(errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return FF_TCP_ERR;
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        fftcpSetError(err, "bind: %s(errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return FF_TCP_ERR; 
    }

    if (listen(sockfd, FF_TCP_BACKLOG) == -1) {
        fftcpSetError(err, "listen: %s(errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return FF_TCP_ERR; 
    }
    
    return sockfd;
}

int fftcpAccept(char *err, int sockfd, char *ip, uint16_t *port) {
    int acptfd;

    struct sockaddr sa;
    socklen_t saLen = sizeof(struct sockaddr);
    
    if ((acptfd = accept(sockfd, &sa, &saLen)) == -1) {
        fftcpSetError(err, "accept: %s(errno: %d)\n", strerror(errno), errno);
        return FF_TCP_ERR;
    }

    if (sa.sa_family != AF_INET) {
        fftcpSetError(err, "[ERROR] Only Support IPv4\n");
        close(acptfd);
        return FF_TCP_ERR;
    }
    struct sockaddr_in *sai = (struct sockaddr_in *) &sa;
    if (ip) strcpy(ip, inet_ntoa(sai->sin_addr));
    if (port) *port = ntohs(sai->sin_port);

    return acptfd;
}
