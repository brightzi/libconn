#include "io.h"
#include "event_loop.h"
#include "cssl.h"
#include "ctimer.h"
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <openssl/ssl.h>

#define DEFAULT_CONNECT_TIMEOUT 5000

void make_noblock_fd(int fd) {
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

int create_socket(int non_block) {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (non_block) {
        make_noblock_fd(client_fd);
    }
    return client_fd; 
}

void close_socket(io_t io) {
    close(io->fd);
    io->fd = -1;
}

static void __connect_timeout_cb(event_timer_t timer) {
    printf("connect timeout\n");
    io_t io = (io_t)timer->privdata;
    io_close(io);
}

static void io_connect_cb(io_t io) {
    io->last_read_time = get_curtime_ms();
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(struct sockaddr_in));
    int peerLen = sizeof(struct sockaddr_in);
    int ret = getpeername(io->fd, (struct sockaddr *)&servAddr, &peerLen);
    if (ret < 0) {
        io_close(io);
        return ;
    }
    char ipAddr[INET_ADDRSTRLEN];
    printf("connected peer address = %s:%d\n", inet_ntop(AF_INET, &servAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(servAddr.sin_port));

    io_del(io, EVENT_WRITE);
    if (io->type == IO_TYPE_SSL) {
        if (io->ssl_ctx == NULL) {
            io->ssl_ctx = create_ssl_context(CSSL_CLIENT);
            io->ssl = ssl_new(io->ssl_ctx, io->fd);
            if (ssl_connect(io->ssl) < 0) {
                free_io(io);
                return ;
            } else {
                if (io->connect_cb) {
                    io->connect_cb(io);
                    ssl_get_peer_cert_chain(io->ssl);
                    io_read_enable(io);
                }
            }
        }
    } else {
        if (io->connect_cb) {
            io->connect_cb(io);
            io_read_enable(io);
        }
    }
}

static int ssl_server_handshake(io_t io) {
    int ret = ssl_accept(io->ssl);
    if (ret == 0) {
        if (io->accept_cb) {
            io->accept_cb(io);
        }
        io_read_enable(io);
        return 0;
    } else if (ret == CSSL_WANT_READ) {
        if ((io->events & EVENT_READ) == 0) {
            io_add(io, ssl_server_handshake, EVENT_READ);
        }
    } else {
        io_close(io);
        return -1;
    }
    return -1;
}

static void io_accept_cb(io_t io) {
    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(struct sockaddr_in));
    int conn_fd = accept(io->fd, (struct sockaddr *)&clientAddr, (socklen_t *)&clientAddr);
    if (conn_fd < 0) {
        return ;
    }
    make_noblock_fd(conn_fd);
     
    io_t conn_io = get_io(io->loop, conn_fd);
    conn_io->accept_cb = io->accept_cb;
    conn_io->userdata = io->userdata;
    if (io->type == IO_TYPE_SSL) {
        if (conn_io->ssl_ctx == NULL && conn_io->ssl == NULL) {
            conn_io->type = IO_TYPE_SSL;
            conn_io->ssl_ctx = create_ssl_context(CSSL_SERVER);
            configure_context(conn_io->ssl_ctx);
            conn_io->ssl = ssl_new(conn_io->ssl_ctx, conn_io->fd);
            ssl_server_handshake(conn_io);
        }
    } else {
        io_read_enable(conn_io);
        if (conn_io->accept_cb) {
            conn_io->accept_cb(conn_io);
        }
    }
}

bool buffer_is_empty(buffer_t buf) {
    return buf->head == buf->tail;
}

void io_read_cb(io_t io) {
    io->last_read_time = io->loop->cur_ms;
    char *buf = io->read_buf->base + io->read_buf->tail;
    int len = io->read_buf->len - io->read_buf->tail;
    int nread;
    if (io->type == IO_TYPE_SSL) {
        nread = ssl_read(io->ssl, buf, len);
    }  else {
        nread = read(io->fd, buf, len);
    }
    
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
            return ;
        }
    } else if (nread == 0) {
        io_close(io);
        return ;
    }

    io->read_buf->tail += nread;
    // if (nread < io->read_buf->len) {
    //     buf[nread] = '\0';
    // }

    if (io->read_cb) {
        io->read_cb(io, buf, nread);
        io->read_buf->tail = 0;
        io->read_buf->head = 0;
    }
    return ;
}

void io_write_cb(io_t io) {
    pthread_mutex_lock(&io->write_mutex);
    io->last_write_time = get_curtime_ms();
    int nwrite = 0;
    if (!buffer_is_empty(io->write_buf)) {
        if (io->type == IO_TYPE_SSL) {
            int ssl_err = SSL_get_error(io->ssl, nwrite);
            nwrite = ssl_write(io->ssl, io->write_buf->base + io->write_buf->head, io->write_buf->tail - io->write_buf->head);
            if (nwrite < 0) {
                if (ssl_err != SSL_ERROR_WANT_READ && ssl_err != SSL_ERROR_WANT_WRITE) {
                    io_close(io);
                }
                pthread_mutex_unlock(&io->write_mutex);
                return ;
            }
            if (io->write_cb) {
                io->write_cb(io, io->write_buf->base + io->write_buf->head, io->write_buf->head + nwrite);
            }
            if (nwrite == io->write_buf->tail - io->write_buf->head) {
                io->write_buf->head = 0;
                io->write_buf->tail = 0;
            } else {
                io->write_buf->head += nwrite;
            }
            pthread_mutex_unlock(&io->write_mutex);
            return ;
        } else {
            nwrite = write(io->fd, io->write_buf->base + io->write_buf->head, io->write_buf->tail - io->write_buf->head);
            if (nwrite < 0) {
                if (errno == EAGAIN || errno == EINTR) {
                    pthread_mutex_unlock(&io->write_mutex);
                    return ;
                } else {
                    io_close(io);
                    pthread_mutex_unlock(&io->write_mutex);
                    return ;
                }
            } else {
                if (io->write_cb) {
                    io->write_cb(io, io->write_buf->base + io->write_buf->head, io->write_buf->head + nwrite);
                }
                if (nwrite == io->write_buf->tail - io->write_buf->head) {
                    io->write_buf->head = 0;
                    io->write_buf->tail = 0;
                } else {
                    io->write_buf->head += nwrite;
                }
            }
        }
    }
    pthread_mutex_unlock(&io->write_mutex);
    return ;
}

void handle_event(io_t io) {
    if ((io->events & EVENT_WRITE) && (io->revents & EVENT_WRITE)) {
        if (io->connect) {
            io->connect = 0;
            io_connect_cb(io);
        } else {
            io_write_cb(io);
        }
    }

    if ((io->events & EVENT_READ) && (io->revents & EVENT_READ)) {
        if (io->accept) {
            io_accept_cb(io);
        } else {
            io_read_cb(io);
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
        io_connect_cb(io);
        return 0;
    }

    io_set_connect_timeout(io, DEFAULT_CONNECT_TIMEOUT);
    io->connect = 1;
    return io_add(io, handle_event, EVENT_WRITE);
}

static int buffer_append_data(io_t io, const void *buf, size_t len) {
    if (io->write_buf->tail + len > io->write_buf->maxSize) {
        return -1;
    }
    memcpy(io->write_buf->base + io->write_buf->tail, buf, len);
    io->write_buf->tail += len;
    return 0;
}

void __write_cb(io_t io, const void *buf, size_t len) {
    if (io->write_cb) {
        io->write_cb(io, buf, len);
        // printf("write cb, buf=%s, len=%d\n", (char *)(buf+2), len);
    }
}

int io_write(io_t io, const void *buf, size_t len) {
    int nwrite = 0;
    pthread_mutex_lock(&io->write_mutex);
    if (!buffer_is_empty(io->write_buf)) {
        if (buffer_append_data(io, buf, len) != 0) {
            io_close(io);
            pthread_mutex_unlock(&io->write_mutex);
            return -1;
        }
        pthread_mutex_unlock(&io->write_mutex);
        return nwrite;
    }
    
    if (io->type == IO_TYPE_SSL) {
        nwrite = ssl_write(io->ssl, buf, len);
        if (nwrite < 0) {
            int ssl_err = SSL_get_error(io->ssl, nwrite);
            if (ssl_err != SSL_ERROR_WANT_READ && ssl_err != SSL_ERROR_WANT_WRITE) {
                io_close(io);
            }
            pthread_mutex_unlock(&io->write_mutex);
            return nwrite;
        } else if (nwrite < len) {
            __write_cb(io, buf + nwrite, len - nwrite);
            if (buffer_append_data(io, buf + nwrite, len - nwrite) != 0) {
                io_close(io);
                pthread_mutex_unlock(&io->write_mutex);
                return nwrite;
            }
            io_add(io, io_write_cb, EVENT_WRITE);
            return nwrite;
        } else {
            __write_cb(io, buf, len);
            pthread_mutex_unlock(&io->write_mutex);
            return nwrite;
        }
    } else {
        if (io->fd == -1) {
            pthread_mutex_unlock(&io->write_mutex);
            return -1;
        }
        nwrite = write(io->fd, buf, len);
        if (nwrite < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                nwrite = 0;
                if (buffer_append_data(io, buf + nwrite, len - nwrite) != 0) {
                    io_close(io);
                    pthread_mutex_unlock(&io->write_mutex);
                    return nwrite;
                }
                io_add(io, io_write_cb, EVENT_WRITE);
                pthread_mutex_unlock(&io->write_mutex);
                return nwrite;
            } else {
                io_close(io);
                pthread_mutex_unlock(&io->write_mutex);
                return -1;
            }
        } else if (nwrite < len) {
            __write_cb(io, buf + nwrite, len - nwrite);
            if (buffer_append_data(io, buf + nwrite, len - nwrite) != 0) {
                io_close(io);
                pthread_mutex_unlock(&io->write_mutex);
                return nwrite;
            }
            io_add(io, io_write_cb, EVENT_WRITE);
        } else {
            __write_cb(io, buf, len);
        }
    }

    if (nwrite > 0) {
        io->last_write_time = get_curtime_ms();
    }
    pthread_mutex_unlock(&io->write_mutex);
    return nwrite;
} 

int io_set_reuse_addr(io_t io) {
    int opt = 1;
    if (setsockopt(io->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return -1;
    }
    return 0;
}

int io_bind(io_t io) {
    struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(atoi(io->port));
    addr->sin_addr.s_addr = inet_addr(io->ip);

    if (bind(io->fd, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) < 0) {
        return -1;
    }
    io->local_addr = addr;
    return 0;
}

int io_listen(io_t io) {
    if (listen(io->fd, 10) < 0) {
        return -1;
    }
    return 0;
}

int io_accept(io_t io) {
    io->accept = 1;
    io_add(io, handle_event, EVENT_READ);
    return 0;
}

void __read_timeout_cb(event_timer_t timer) {
    if (timer == NULL) {
        return ;
    }

    printf("read timeout\n");
    io_t io = (io_t)(timer->privdata);
    event_loop_t loop = io->loop;
    uint64_t inactive_ms = (io->loop->cur_ms - io->last_read_time);
    if (inactive_ms < io->read_timeout) {
        reset_timer(timer, io->read_timeout);
    } else {
        io_close(io);
    }
}

void __writer_timeout_cb(event_timer_t timer) {
    if (timer == NULL) {
        return ;
    }

    printf("write timeout\n");
    io_t io = (io_t)(timer->privdata);
    event_loop_t loop = io->loop;
    uint64_t inactive_ms = (io->loop->cur_ms - io->last_write_time);
    if (inactive_ms < io->write_timeout) {
        reset_timer(timer, io->write_timeout);
    } else {
        io_close(io);
    }
}

void __close_timeout_cb(event_timer_t timer) {
    if (timer == NULL) {
        return ;
    }

    printf("close timeout\n");
    io_t io = (io_t)(timer->privdata);
    event_loop_t loop = io->loop;
    io_close(io);
}

void io_set_read_timeout(io_t io, int timeout) {
    if (timeout < 0) {
        io_remove_read_timeout(io);
        return ;
    }
    if (io->read_timer) {
        reset_timer(io->read_timer, timeout);
    } else {
        io->read_timeout = timeout;
        io->read_timer = add_timer(io->loop, timeout, __read_timeout_cb, 1);
        io->read_timer->privdata = io; 
    }

}

void io_set_write_timeout(io_t io, int timeout) {
    if (timeout < 0) {
        io_remove_write_timeout(io);
        return ;
    }

    if (io->write_timer) {
        reset_timer(io->write_timer, timeout);
    } else {
        io->write_timeout = timeout;
        io->write_timer = add_timer(io->loop, timeout, __writer_timeout_cb, 1);
        io->write_timer->privdata = io;
    }
}

void io_set_close_timeout(io_t io, int timeout) {
    if (timeout < 0) {
        return ;
    }

    if (io->close_timer) {
        reset_timer(io->close_timer, timeout);
    } else {
        io->close_timeout = timeout;
        io->close_timer = add_timer(io->loop, timeout, __close_timeout_cb, 1);
        io->close_timer->privdata = io;
    }
}

void io_set_connect_timeout(io_t io, int timeout) {
    if (timeout < 0) {
        return ;
    }
    if (io->connect_timer) {
        reset_timer(io->connect_timer, timeout);
    } else {
        io->connect_timeout = timeout;
        io->connect_timer = add_timer(io->loop, timeout, __connect_timeout_cb, 1);
        io->connect_timer->privdata = io;
    }
}

void io_remove_read_timeout(io_t io) {
    if (io->read_timer) {
        heap_remove(io->loop->timers, &io->read_timer->node);
        io->read_timeout = 0;
        io->read_timer = NULL;
    }
}

void io_remove_write_timeout(io_t io) {
    if (io->write_timer) {
        heap_remove(io->loop->timers, &io->write_timer->node);
        io->write_timeout = 0;
        io->write_timer = NULL;
    }
}

void io_remove_close_timeout(io_t io) {
    if (io->close_timer) {
        heap_remove(io->loop->timers, &io->close_timer->node);
        io->close_timeout = 0;
        io->close_timer = NULL;
    }
}

void io_remove_connect_timeout(io_t io) {
    if (io->connect_timer) {
        heap_remove(io->loop->timers, &io->connect_timer->node);
        io->connect_timeout = 0;
        io->connect_timer = NULL;
    }
}
