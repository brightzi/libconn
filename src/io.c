#include "io.h"
#include "event_loop.h"
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_TIMEOUT 10000

static void make_noblock_fd(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


int io_add(io_t io, event_cb cb, int event) {
    if (io == NULL) {
        return -1;
    }

    io->cb = cb;
    if (!(io->events & event)) {
        io->events |= event;
        io->loop->disp->add(io->loop, io->fd, event);
    }
    return 0;
}

int io_del(io_t io, int event) {
    if (io == NULL) {
        return -1;
    }

    if (io->events & event) {
        io->events &= ~event;
        io->loop->disp->del(io->loop, io->fd, event);
    }
    return 0;
}

int create_socket(int block) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (block) {
        make_noblock_fd(client_fd);
    }
    return client_fd; 
}

void close_socket(io_t io) {
    close(io->fd);
    io->fd = 0;
}

void close_io(io_t io) {

}

static void __timeout_cb(event_timer_t timer) {
    io_t io = (io_t)timer->privdata;
    close_io(io);
}

static void __connect_cb(io_t io) {
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(struct sockaddr_in));
    int peerLen = sizeof(struct sockaddr_in);
    int ret = getpeername(io->fd, (struct sockaddr *)&servAddr, &peerLen);
    if (ret < 0) {
        if (io->close_cb) {
            io->close_cb(io);
        }
        io_del(io, EVENT_WRITE);
        free_io(io);
        return ;
    }
    char ipAddr[INET_ADDRSTRLEN];
    printf("connected peer address = %s:%d\n", inet_ntop(AF_INET, &servAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(servAddr.sin_port));

    io_del(io, EVENT_WRITE);
    if (io->type == IO_TYPE_SSL) {
        if (io->ssl_ctx == NULL) {

        }
    } else {
        if (io->connect_cb) {
            io->connect_cb(io);
            io_read_enable(io);
        }
    }
}

void __read_cb(io_t io) {
    char *buf = io->read_buf->base + io->read_buf->tail;
    int len = io->read_buf->len - io->read_buf->tail;
    int nread = read(io->fd, buf, len);
    if (nread + io->read_buf->tail == io->read_buf->len) {
        ///关闭链接
        io_close(io);
        return ;
    }

    if (nread < 0) {
        if (errno == EAGAIN) {
            return;
        } else {
            io_close(io);
        }
    } else if (nread == 0) {
        io_close(io);
        return ;
    }

    io->read_buf->tail += nread;
    if (nread < io->read_buf->len) {
        buf[nread] = '\0';
    }

    if (io->read_cb) {
        io->read_cb(io, buf, nread);
        io->read_buf->tail = 0;
        io->read_buf->head = 0;
    }
    return ;
}

void handle_event(io_t io) {
    if ((io->events & EVENT_WRITE) && (io->revents & EVENT_WRITE)) {
        if (io->connect) {
            io->connect = 0;
            __connect_cb(io);
        } else {
            if (io->write_cb) {
                io->write_cb(io, NULL, 0);
            }
        }
    }

    if ((io->events & EVENT_READ) && (io->revents & EVENT_READ)) {
        if (io->type == IO_TYPE_PIPE) {
            io->read_cb(io, NULL, 0);
        } else if (io->type == IO_TYPE_TCP) {
            __read_cb(io);
        }
    }
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
        __connect_cb(io);
        return 0;
    }

    io->connect_timeout = io->connect_timeout ? io->connect_timeout : DEFAULT_TIMEOUT;
    io->connect_timer = add_timer(io->loop, io->connect_timeout, __timeout_cb, 1);
    io->connect_timer->privdata = io;
    io->connect = 1;
    return io_add(io, handle_event, EVENT_WRITE);
}

int io_write(io_t io, const void *buf, size_t len) {
    return write(io->fd, buf, len);
} 