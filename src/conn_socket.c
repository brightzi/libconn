#include "conn_socket.h"
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>

static void nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void closesocket(fd) {
    close(fd);
}

int connectWithTimeout(const char *host, int port, int timeout) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return -1;
    }

    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int ret = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret < 0 && errno != EINPROGRESS) {
        perror("connect");
        close(sock);
        return -1;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);

    ret = select(sock + 1, NULL, &writefds, NULL, &tv);

    if (ret < 0) {
        closesocket(sock);
        return -1;
    } else if (ret == 0) {
        closesocket(sock);
        return -1;
    }

    int err = 0;
    socklen_t optlen = sizeof(err);

    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&err, &optlen) < 0 || err != 0) {
        if (err != 0) errno = err;
        closesocket(sock);
        return -1;
    }

    return sock;
}