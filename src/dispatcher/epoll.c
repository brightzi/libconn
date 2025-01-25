#include <sys/epoll.h>
#include <stddef.h>
#include <errno.h>
#include "dispatcher.h"

typedef struct epoll_dispatcher_st epoll_dispatcher_st, *epoll_dispatcher_t;

struct epoll_dispatcher_st {
    int epfd;
    struct epoll_event *events;
    size_t events_capacity;
};

static void *epoll_init();
static int epoll_add(event_loop_t loop, int fd, int event);
static int epoll_update(event_loop_t loop, int fd, int event);
static int epoll_del(event_loop_t loop, int fd, int event);
static int epoll_run(event_loop_t loop, int timeout);
static void epoll_destory(event_loop_t loop);

const struct event_dispatcher epoll_dispatcher = {
    "epoll",
    epoll_init,
    epoll_add,
    epoll_update,
    epoll_del,
    epoll_run,
    epoll_destory,
};

void *epoll_init() {
    const size_t INITIAL_EVENTS_CAPACITY = 1024;
    
    epoll_dispatcher_t dispatcher = calloc(sizeof(epoll_dispatcher_st), 1);
    if (dispatcher == NULL) {
        return NULL;
    }
    
    dispatcher->epfd = epoll_create1(0);
    if (dispatcher->epfd < 0) {
        free(dispatcher);
        return NULL;
    }

    dispatcher->events_capacity = INITIAL_EVENTS_CAPACITY;
    dispatcher->events = calloc(sizeof(struct epoll_event), dispatcher->events_capacity);
    if (dispatcher->events == NULL) {
        close(dispatcher->epfd);
        free(dispatcher);
        return NULL;
    }

    return (void *)dispatcher;
}

static int epoll_add(event_loop_t loop, int fd, int event) {
    epoll_dispatcher_t disp = (epoll_dispatcher_t)loop->disp_data;
    if (!disp) {
        return -1;
    }

    struct epoll_event ev;
    ev.events = 0;
    ev.data.fd = fd;

    if (event & EVENT_READ) {
        ev.events |= EPOLLIN;
    }
    if (event & EVENT_WRITE) {
        ev.events |= EPOLLOUT;
    }

    if (epoll_ctl(disp->epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        return -1;
    }
    return 0;
}

static int epoll_update(event_loop_t loop, int fd, int event) {
    epoll_dispatcher_t disp = (epoll_dispatcher_t)loop->disp_data;
    if (!disp) {
        return -1;
    }

    struct epoll_event ev;
    ev.events = 0;
    ev.data.fd = fd;

    if (event & EVENT_READ) {
        ev.events |= EPOLLIN;
    }
    if (event & EVENT_WRITE) {
        ev.events |= EPOLLOUT;
    }

    if (epoll_ctl(disp->epfd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        return -1;
    }
    return 0;
}

static int epoll_del(event_loop_t loop, int fd, int event) {
    epoll_dispatcher_t disp = (epoll_dispatcher_t)loop->disp_data;
    if (!disp) {
        return -1;
    }

    struct epoll_event ev;
    ev.events = 0;
    ev.data.fd = fd;

    if (epoll_ctl(disp->epfd, EPOLL_CTL_DEL, fd, &ev) < 0) {
        return -1;
    }
    return 0;
}

static int epoll_run(event_loop_t loop, int timeout) {
    epoll_dispatcher_t disp = (epoll_dispatcher_t)loop->disp_data;
    if (!disp) {
        return -1;
    }

    int nfds = epoll_wait(disp->epfd, disp->events, disp->events_capacity, timeout);
    if (nfds < 0) {
        if (errno != EINTR) {
            return -1;
        }
        return 0;
    }

    if (nfds == 0) {
        return 0;
    }

    if (nfds >= disp->events_capacity * 0.8) {
        size_t new_capacity = disp->events_capacity * 2;
        struct epoll_event *new_events = realloc(disp->events, 
            sizeof(struct epoll_event) * new_capacity);
        
        if (new_events) {
            disp->events = new_events;
            disp->events_capacity = new_capacity;
        }
    }

    for (int i = 0; i < nfds; i++) {
        int fd = disp->events[i].data.fd;
        int revents = 0;

        if (disp->events[i].events & EPOLLIN) {
            revents |= EVENT_READ;
        }
        if (disp->events[i].events & EPOLLOUT) {
            revents |= EVENT_WRITE;
        }

        if (revents > 0) {
            io_t io = loop->io_array[fd];
            if (io) {
                io->revents = revents;
                event_pending(io);
            }
        }
    }

    return nfds;
}

static void epoll_destory(event_loop_t loop) {
    epoll_dispatcher_t disp = (epoll_dispatcher_t)loop->disp_data;
    if (disp) {
        if (disp->events) {
            free(disp->events);
        }
        close(disp->epfd);
        free(disp);
    }
    return;
}
