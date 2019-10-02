#ifndef FF_TCP_H
#define FF_TCP_H

#define FF_TCP_ERR -1
#define FF_TCP_ERR_LEN 128
#define FF_TCP_BACKLOG 1000

int fftcpServer(char *err, char *port, char *bindaddr);
int fftcpAccept(char *err, int sockfd, char *ip, uint16_t *port);

#endif /* FF_TCP_H */
