#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "dispatcher.h"

typedef struct poll_dispatcher_st poll_dispatcher_st, *poll_dispatcher_t;

typedef struct poll_dispatcher_st {
    struct pollfd *fds;
    size_t fds_count;
    size_t fds_capacity;
} poll_dispatcher_st, *poll_dispatcher_t;

static void *poll_init();
static int poll_add(event_loop_t loop, int fd, int event);
static int poll_update(event_loop_t loop, int fd, int event);
static int poll_del(event_loop_t loop, int fd, int event);
static int poll_run(event_loop_t loop, int timeout);
static void poll_destroy(event_loop_t loop);

const struct event_dispatcher poll_dispatcher = {
    "poll",
    poll_init,
    poll_add,
    poll_update,
    poll_del,
    poll_run,
    poll_destroy,
};

void *poll_init() {
    const size_t INITIAL_FDS_CAPACITY = 1024;
    
    poll_dispatcher_t dispatcher = calloc(1, sizeof(poll_dispatcher_st));
    if (dispatcher == NULL) {
        return NULL;
    }

    dispatcher->fds = calloc(INITIAL_FDS_CAPACITY, sizeof(struct pollfd));
    if (dispatcher->fds == NULL) {
        free(dispatcher);
        return NULL;
    }

    dispatcher->fds_capacity = INITIAL_FDS_CAPACITY;
    return (void *)dispatcher;
}

static int poll_add(event_loop_t loop, int fd, int event) {
    poll_dispatcher_t disp = (poll_dispatcher_t)loop->disp_data;
    if (!disp || fd < 0) {
        return -1;
    }

    if (disp->fds_count >= disp->fds_capacity) {
        size_t new_capacity = disp->fds_capacity * 2;
        struct pollfd *new_fds = realloc(disp->fds, new_capacity * sizeof(struct pollfd));
        if (new_fds == NULL) {
            return -1;
        }
        disp->fds = new_fds;
        disp->fds_capacity = new_capacity;
    }

    disp->fds[disp->fds_count].fd = fd;
    disp->fds[disp->fds_count].events = 0;
    if (event & EVENT_READ) {
        disp->fds[disp->fds_count].events |= POLLIN;
    }
    if (event & EVENT_WRITE) {
        disp->fds[disp->fds_count].events |= POLLOUT;
    }

    disp->fds_count++;
    return 0;
}

static int poll_update(event_loop_t loop, int fd, int event) {
    poll_dispatcher_t disp = (poll_dispatcher_t)loop->disp_data;
    if (!disp || fd < 0) {
        return -1;
    }

    for (size_t i = 0; i < disp->fds_count; i++) {
        if (disp->fds[i].fd == fd) {
            disp->fds[i].events = 0;
            if (event & EVENT_READ) {
                disp->fds[i].events |= POLLIN;
            }
            if (event & EVENT_WRITE) {
                disp->fds[i].events |= POLLOUT;
            }
            return 0;
        }
    }
    
    return -1; // fd not found
}

static int poll_del(event_loop_t loop, int fd, int event) {
    poll_dispatcher_t disp = (poll_dispatcher_t)loop->disp_data;
    if (!disp || fd < 0) {
        return -1;
    }

    for (size_t i = 0; i < disp->fds_count; i++) {
        if (disp->fds[i].fd == fd) {
            if (i < disp->fds_count - 1) {
                disp->fds[i] = disp->fds[disp->fds_count - 1]; // Move last to current
            }
            disp->fds_count--;
            return 0;
        }
    }
    
    return -1; // fd not found
}

static int poll_run(event_loop_t loop, int timeout) {
    poll_dispatcher_t disp = (poll_dispatcher_t)loop->disp_data;
    if (!disp) {
        return -1;
    }

    int nfds = poll(disp->fds, disp->fds_count, timeout);
    if (nfds < 0) {
        if (errno != EINTR) {
            return -1;
        }
        return 0;
    }

    for (size_t i = 0; i < disp->fds_count && nfds > 0; i++) {
        if (disp->fds[i].revents) {
            nfds--;
            int revents = 0;
            if (disp->fds[i].revents & POLLIN) {
                revents |= EVENT_READ;
            }
            if (disp->fds[i].revents & POLLOUT) {
                revents |= EVENT_WRITE;
            }

            if (revents > 0) {
                int fd = disp->fds[i].fd;
                io_t io = loop->io_array[fd];
                if (io) {
                    io->revents = revents;
                    event_pending(io);
                }
            }
        }
    }
  
    return 0;
}

static void poll_destroy(event_loop_t loop) {
    poll_dispatcher_t disp = (poll_dispatcher_t)loop->disp_data;
    if (disp) {
        if (disp->fds) {
            free(disp->fds);
        }
        free(disp);
    }
    return;
}