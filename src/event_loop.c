#include "event_loop.h"
#include "timer.h"
#include "dispatcher.h"

extern const struct event_dispatcher select_dispatcher;

event_loop_t event_loop_init() {
    return event_loop_init_with_name(NULL);
}

void pipe_read_cb(io_t io, void *bufff, int readybytes) {
    char buf[1024];
    read(io->fd, buf, 1024);
    printf("buf : %s\n", buf);
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
    create_io(loop, loop->pipefd[0], EVENT_READ, pipe_read_cb, NULL); 
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
        loop->quit = 0;
    }
}

int event_loop_wakeup(event_loop_t loop) {
    if (!loop) {
        return -1;
    }   
    write(loop->pipefd[1], "hello world", strlen("hello world")+1);
    return 0;
}

void process_pending_event(event_loop_t loop) {
    if (!loop) {
        return -1;
    }

    while(loop->pending) {
        io_t io = loop->pending;
        if (io->revents & EVENT_READ) {
            if (io->read_cb) {
                io->read_cb(io, NULL, 0);
            }
        }
        if (io->revents & EVENT_WRITE) {
            if (io->write_cb) {
                io->write_cb(io, NULL, 0);
            }
        }
    }

    loop->pending = NULL;
    return ;
}


static void io_init(io_t io) {

}

io_t create_io(event_loop_t loop, int fd, int events, read_cb read_cb, write_cb write_cb) {
    if (!loop) {
        return NULL;
    }

    io_t io = malloc(sizeof(io_st));
    if (io == NULL) {
        return NULL;
    }

    memset(io, 0, sizeof(io_st));
    io_init(io);
    io->loop = loop;
    io->fd = fd;
    io->read_cb = read_cb;
    io->write_cb = write_cb;
    loop->io_array[fd] = io;
    loop->io_num++;
    return io;
}


void free_io(io_t io) {
    //TODO
    //remove io read/wrtie from dispatcher
    // close socket
    //free io
    if (io) {
        free(io);
    }
}

