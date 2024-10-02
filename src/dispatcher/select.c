#include <sys/select.h>
#include <stddef.h>
#include <errno.h>
#include "dispatcher.h"

typedef struct select_dispatcher_st select_dispatcher_st, *select_dispatcher_t;

struct select_dispatcher_st{
    int max_fd;
    fd_set read_fds;
    fd_set write_fds;
};

static void *select_init();

static int select_add(event_loop_t loop, int fd, int event);

static int select_update(event_loop_t loop, int fd, int event);

static int select_del(event_loop_t loop, int fd, int event);

static int select_run(event_loop_t loop, int timeout);
    
static void select_destory(event_loop_t loop);

const struct event_dispatcher select_dispatcher = {
        "select",
        select_init,
        select_add,
        select_update,
        select_del,
        select_run,
        select_destory,
};

void *select_init() {
    select_dispatcher_t dispatcher = calloc(sizeof(select_dispatcher_st), 1);
    if (dispatcher == NULL) {
        return NULL;
    }
    FD_ZERO(&dispatcher->read_fds);
    FD_ZERO(&dispatcher->write_fds);
    dispatcher->max_fd = -1;
    return (void *)dispatcher;
}

int select_add(event_loop_t loop, int fd, int event) {
    select_dispatcher_t disp = (select_dispatcher_t) loop->disp_data;
    if (!disp) {
        return -1;
    }
    if (event & EVENT_READ) {
        if (!FD_ISSET(fd, &disp->read_fds)) {
            FD_SET(fd, &disp->read_fds);
        }
    } 

    if (event & EVENT_WRITE) {
        if (!FD_ISSET(fd, &disp->write_fds)) {
            FD_SET(fd, &disp->write_fds);
        }
    }

    if (fd > disp->max_fd) {
        disp->max_fd = fd;
    }
    return 0;
}

int select_update(event_loop_t loop, int fd, int event) {
    select_dispatcher_t disp = (select_dispatcher_t) loop->disp_data;
    if (!disp) {
        return -1;
    }
    return select_add(loop, fd, event);
}

int select_del(event_loop_t loop, int fd, int event) {
    select_dispatcher_t disp = (select_dispatcher_t) loop->disp_data;
    if (!disp) {
        return -1;
    }

    if (event & EVENT_READ) {
        FD_CLR(fd, &disp->read_fds);
    }
    if (event & EVENT_WRITE) {
        FD_CLR(fd, &disp->write_fds);
    }
    return 0;
}

int select_run(event_loop_t loop, int timeout) {
    select_dispatcher_t disp = (select_dispatcher_t) loop->disp_data;
    if (!disp) {
        return -1;
    }
    fd_set read_fds = disp->read_fds;
    fd_set write_fds = disp->write_fds;
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    int ret = select(disp->max_fd + 1, &read_fds, &write_fds, NULL, &tv);
    if (ret < 0) {
        if (errno != EINTR) {
            printf("select errno\n");
        }
        //todo 错误处理
        return -1;
    }

    if (ret == 0) {
        return 0;
    }

    int revents = 0;
    int nevents = 0;
    for(int i = 0; i < disp->max_fd + 1; i++) {
        revents = 0;
        if (FD_ISSET(i, &read_fds)) {
            ++nevents; 
            revents |= EVENT_READ;
        }

        if (FD_ISSET(i, &write_fds)) {
            ++nevents;
            revents |= EVENT_WRITE;
        }

        if (revents > 0) {
            io_t io = loop->io_array[i];
            io->revents = revents;
            if (io) {
                event_pending(io);
            }
        }
    }

    return ret;
}

void select_destory(event_loop_t loop) {
    select_dispatcher_t disp = (select_dispatcher_t) loop->disp_data;
    if (disp) {
        free(disp);
    }
    return ;
}

