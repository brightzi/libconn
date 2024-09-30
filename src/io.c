#include "io.h"
#include <fcntl.h>
#include <errno.h>

void make_noblock_fd(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


void io_add(io_t io, io_cb cb, int event) {
    if (io == NULL) {
        return ;
    }

    io->cb = cb;
    if (!(io->events & event)) {
        io->events |= event;
        io->loop->disp->add(io->loop, io->fd, event);
    }
    return ;
}

void io_del(io_t, int event) {

}

int create_socket() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    make_noblock_fd(client_fd);
    return client_fd; 
}

void close_socket(io_t io) {
    close(io->fd);
    io->fd = 0;
}

int io_connect(io_t io) {
    struct sockaddr_in *serverAddr;
    serverAddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    if (serverAddr == NULL) {
        return -1;
    }

    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(atoi(io->port));
    serverAddr->sin_addr.s_addr = inet_addr(io->ip);
    
    int ret = connect(io->fd, (struct sockaddr *)serverAddr, sizeof(struct sockaddr_in));
    if (ret < 0 && errno != EINPROGRESS) {
        return -1;
    } else if (ret == 0) {

    }

    return 0;
}