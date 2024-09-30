#include "event_loop.h"
#include "timer.h"
#include "dispatcher.h"
#include "io.h"

extern const struct event_dispatcher select_dispatcher;

event_loop_t event_loop_init() {
    return event_loop_init_with_name(NULL);
}

void pipe_read_cb(io_t io, void *bufff, int readybytes) {
    char buf[1024];
    int nread;
    while ((nread = read(io->fd, buf, 1024)) > 0) {
        printf("buf : %s\n", buf);
        if (nread == 0) {
            if (errno == EAGAIN) {
                printf("read again\n");
                break;
            }
        } else if (nread > 0) {
            break;
        } else {
            break;
        }
    }
}

event_loop_t event_loop_init_with_name(const char *name) {
    event_loop_t loop = malloc(sizeof(event_loop_st));
    if (loop == NULL) {
        return NULL;
    }
    loop->quit = 0;
    loop->tid = pthread_self();
    if (!name) {
        snprintf(loop->name, sizeof(loop->name), "mainloop-%d", loop->tid);
    }
    loop->start_ms = get_curtime_ms();
    loop->cur_ms = get_curtime_ms();
    io_st **io_array = malloc(sizeof(io_st *) * 1024);
    if (io_array == NULL) {
        free(loop);
        return NULL;
    }
    loop->io_array = io_array;
    loop->io_maxsize = 1024;
    loop->pending = NULL;
    loop->npendings = 0;
    
    pipe(loop->pipefd);

    loop->disp = &select_dispatcher;
    loop->disp_data = loop->disp->init(loop);
    io_t io = create_io(loop, loop->pipefd[0], EVENT_READ, pipe_read_cb, NULL); 
    loop->disp->add(loop, loop->pipefd[0], EVENT_READ);
    return loop;    
}

int event_loop_destory(event_loop_t loop) {
    if (loop->io_array) {
        for (int i = 0; i < loop->io_maxsize; i++) {
            if (loop->io_array[i]) {
                free(loop->io_array[i]);
            }
        }
        free(loop->io_array);
    }
    free(loop);
}

int event_loop_run(event_loop_t loop) {
    if (!loop) {
        return -1;
    }
    while (!loop->quit) {
        if (loop->io_num > 0) {
            loop->disp->run(loop, 1000);
        } else {
            sleep_ms(10);
        }
        process_pending_event(loop);
    }
    return 0;
}

int event_loop_stop(event_loop_t loop) {
    if (loop) {
        loop->quit = 1;
    }
}

int event_loop_wakeup(event_loop_t loop) {
    if (!loop) {
        return -1;
    }   
    write(loop->pipefd[1], "hello world", strlen("hello world"));
    write(loop->pipefd[1], "hello world", strlen("hello world"));
    write(loop->pipefd[1], "hello world", strlen("hello world"));
    write(loop->pipefd[1], "hello world", strlen("hello world"));
    write(loop->pipefd[1], "hello world", strlen("hello world"));
    write(loop->pipefd[1], "hello world", strlen("hello world"));
    return 0;
}

void process_pending_event(event_loop_t loop) {
    if (!loop) {
        return -1;
    }

    io_t io = loop->pending;
    while(io) {
        if (io->cb) {
            io->cb(io);
            io->pending = 0;
        }
        io = io->next_event;
    }

    loop->pending = NULL;
    return ;
}


io_t get_io(event_loop_t loop, int fd) {
    io_t io = malloc(sizeof(io_st));
    if (io == NULL) {
        return NULL;
    }
    memset(io, 0, sizeof(io_st));
    io->fd = fd;
    io->loop = loop;
    loop->io_array[fd] = io;
    loop->io_num++;
    return io;
}

io_t create_io(event_loop_t loop, int fd, int events, read_cb read_cb, write_cb write_cb) {
    if (!loop) {
        return NULL;
    }
    io_t io = get_io(loop,fd);
    if (io) {
        io->read_cb = read_cb;
        io->write_cb = write_cb;
    }
    return io;
}


void free_io(io_t io) {
    //TODO
    //remove io read/wrtie from dispatcher
    // close socket
    //free io
    if (io) {
        if (io->fd) {
            close_socket(io);
        }
        free(io);
    }
}

int io_read_enable(io_t io) {
    io_add(io, handle_event, EVENT_READ);
    return 0;
}

void io_set_readcb(io_t io, read_cb cb) {
    io->read_cb = cb;
}

void io_set_writecb(io_t io, write_cb cb) {
    io->write_cb = cb;
}

void io_set_closecb(io_t io, close_cb cb) {
    io->close_cb = cb;
}

void io_set_connectcb(io_t io, connect_cb cb) {
    io->connect_cb = cb;
}

void io_set_acceptcb(io_t io, accept_cb cb) {
    io->accept_cb = cb;
}

event_timer_t add_timer(event_loop_t loop, int timeout, timer_cb cb, int repeat) {
    event_timer_t timer = malloc(sizeof(event_timer_st));
    if (!timer) {
        return NULL;
    } 

    timer->cb = cb;
    timer->repeat = repeat;
    return timer;
}

void del_timer(event_loop_t loop, event_timer_t timer) {

}

io_t create_tcp_client(event_loop_t loop, const char *ip, const char *port, connect_cb connect_cb, close_cb close_cb) {
    if (loop == NULL) {
        return NULL;
    }

    int client_fd = create_socket();
    io_t io = get_io(loop, client_fd);
    if (io == NULL) {
        return NULL;
    }
    io->ip = strdup(ip);
    io->port = strdup(port);
    int ret = io_connect(io);
    if (ret != 0) {
        free_io(io);
        return NULL;
    }

    io->connect_cb = connect_cb;
    io->close_cb = close_cb;
    return io;
}

