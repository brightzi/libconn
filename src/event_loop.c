#include "event_loop.h"
#include "ctimer.h"
#include "dispatcher.h"
#include "io.h"
#include "heap.h"


#define container_of(ptr, type, member) \
((type*)((char*)(ptr) - offsetof(type, member)))

#define TIMER_ENTRY(p)          container_of(p, struct event_timer_st, node)

static int timers_compare(const struct heap_node* lhs, const struct heap_node* rhs) {
    return TIMER_ENTRY(lhs)->next_timeout < TIMER_ENTRY(rhs)->next_timeout;
}

#define MAX_IO_BUF_SIZE 1024 * 1024 * 4

extern const struct event_dispatcher select_dispatcher;

event_loop_t event_loop_init() {
    return event_loop_init_with_name(NULL);
}

void pipe_read_cb(io_t io, void *bufff, int readybytes) {
    pthread_mutex_lock(&io->loop->mutex);
    char buf[1024];
    int nread = read(io->fd, buf, 1024);
    for (int i = 0; i < nread; i++) {
        event_t event = io->loop->custom_events[i]; 
        if (event->cb) {
            event->cb(event);
        }
    }
    io->loop->custom_events_num = 0;
    pthread_mutex_unlock(&io->loop->mutex);
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
    heap_init(&loop->timers, timers_compare);
    
    pipe(loop->pipefd);

    loop->disp = &select_dispatcher;
    loop->disp_data = loop->disp->init(loop);
    // io_t io = create_io(loop, loop->pipefd[0], EVENT_READ, pipe_read_cb, NULL); 
    io_t io = get_io(loop, loop->pipefd[0]);
    io->type = io_pipe;
    io_add(io, handle_event, EVENT_READ);
    io_set_readcb(io, pipe_read_cb);
    loop->disp->add(loop, loop->pipefd[0], EVENT_READ);

    loop->custom_events = malloc(sizeof(event_t) * 1024);
    if (loop->custom_events == NULL) {
        free(loop);
        return NULL;
    }
    loop->custom_events_num = 0;
    loop->max_custom_events_num = 1024;
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

void process_timer(event_loop_t loop) { 

    struct heap *timers = &loop->timers;
    event_timer_t timer = NULL;
    while(timers->root) {
        timer = TIMER_ENTRY(timers->root);
        if (timer->next_timeout > loop->cur_ms) {
            break;
        }

        if (timer->repeat == 0) {
            del_timer(loop, timer);
        } else {
            heap_insert(timers, &timer->node);
        }
        event_pending(timer);
    }
    
    return ;
}

int event_loop_run(event_loop_t loop) {
    if (!loop) {
        return -1;
    }
    while (!loop->quit) {
        int blocktime_ms = 10;
        if (loop->timers_num > 0) {
            update_loop_timer(loop);
            if (loop->timers.root) {
                int64_t min_timeout = TIMER_ENTRY(loop->timers.root)->next_timeout - loop->cur_ms;
                blocktime_ms = min_timeout < blocktime_ms ? min_timeout : blocktime_ms;
            }

            if (blocktime_ms < 0) {
                // process timer
                if (loop->timers_num > 0) {
                    process_timer(loop);
                }
                continue;
            }
        }

        if (loop->io_num > 0) {
            loop->disp->run(loop, blocktime_ms);
        } else {
            sleep_ms(blocktime_ms);
        }
        update_loop_timer(loop);
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
    return 0;
}

void update_loop_timer(event_loop_t loop) {
    loop->cur_ms = get_monotime_ms();
}

void process_pending_event(event_loop_t loop) {
    if (!loop) {
        return -1;
    }

    event_t io = loop->pending;
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
    io->type = io_tcp;
    loop->io_array[fd] = io;
    loop->io_num++;

    io->read_buf = (buffer_t)malloc(sizeof(buffer_st));
    if (io->read_buf == NULL) {
        return NULL;
    }
    io->write_buf = (buffer_t)malloc(sizeof(buffer_st));
    if (io->write_buf == NULL) {
        return NULL;
    }

    char *tmp = (char *)malloc(MAX_IO_BUF_SIZE);
    if (tmp == NULL) {
        return NULL;
    }
    io->read_buf->base = tmp;
    io->read_buf->len = MAX_IO_BUF_SIZE;
    io->read_buf->head = 0;
    io->read_buf->tail = 0;

    tmp = (char *)malloc(MAX_IO_BUF_SIZE);
    if (tmp == NULL) {
        return NULL;
    }

    io->write_buf->base = tmp;
    io->write_buf->len = MAX_IO_BUF_SIZE;
    io->write_buf->head = 0;
    io->write_buf->tail = 0;
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
        free(io);
    }
}

void io_close(io_t io) {
    if (io->closed) {
        return;
    }
    io->closed = 1;
    io->loop->disp->del(io->loop, io->fd, EVENT_READ | EVENT_WRITE);
    if (io->close_cb) {
        io->close_cb(io);
    }
    if (io->fd) {
        close_socket(io);
    }
    free_io(io);
}



int io_send_data(io_t io, const void *buf, size_t len) {
    if (io->closed) {
        return -1;
    }
    return io_write(io, buf, len);
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
    if (timeout == 0) {
        return NULL;
    }

    event_timer_t timer = malloc(sizeof(event_timer_st));
    if (!timer) {
        return NULL;
    } 

    update_loop_timer(loop);
    timer->timeout = timeout;
    timer->next_timeout = loop->cur_ms + timeout;
    timer->cb = cb;
    timer->repeat = repeat;
    timer->loop = loop;
    heap_insert(&loop->timers, &timer->node);
    loop->timers_num++;
    return timer;
}

void del_timer(event_loop_t loop, event_timer_t timer) {
    timer->loop->timers_num--; 
    heap_remove(&loop->timers, &timer->node);
}

io_t create_tcp_client(event_loop_t loop, const char *ip, const char *port, connect_cb connect_cb, close_cb close_cb, void *userdata) {
    if (loop == NULL) {
        return NULL;
    }

    int block = 0;
    int client_fd = create_socket(block);
    io_t io = get_io(loop, client_fd);
    if (io == NULL) {
        return NULL;
    }
    io->ip = strdup(ip);
    io->port = strdup(port);
    io->connect_cb = connect_cb;
    io->close_cb = close_cb;
    io->userdata = userdata;

    int ret = io_connect(io);
    if (ret != 0) {
        free_io(io);
        return NULL;
    }
    return io;
}


void loop_post_event(event_loop_t loop, event_t event) {
    if (loop == NULL) {
        return ;
    }
    if (event == NULL) {
        return ;
    }
    
    pthread_mutex_lock(&loop->mutex);
    int nwrite = write(loop->pipefd[1], "h", 1);
    if (nwrite != 1) {
        pthread_mutex_unlock(&loop->mutex);
        return ;
    }
    loop->custom_events[loop->custom_events_num++] = event;
    pthread_mutex_unlock(&loop->mutex);
    return ;
}
