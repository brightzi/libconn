#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "event.h"

struct event_loop_st;
typedef struct event_loop_st *event_loop_t; 

struct event_dispatcher {
    const char *name;
    
    void *(*init)();

    int (*add)(event_loop_t loop, int fd, int event);

    int (*update)(event_loop_t loop, int fd, int event);

    int (*del)(event_loop_t loop, int fd, int event);

    int (*run)(event_loop_t loop, int timeout);
    
    void (*destory)(event_loop_t loop);
};
#endif