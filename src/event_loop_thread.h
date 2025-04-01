#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include "event_loop.h"

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct event_loop_thread_st event_loop_thread_st, * event_loop_thread_t;

struct event_loop_thread_st {
    event_loop_t eventLoop;
    pthread_t tid;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char *thread_name;
    long thread_count;
};

int event_loop_thread_init(event_loop_thread_t event_loop_thread, int index);

void event_loop_thread_start(event_loop_thread_t event_loop_thread);


#ifdef __cplusplus
}
#endif

#endif