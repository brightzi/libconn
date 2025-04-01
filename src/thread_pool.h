#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "event_loop_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct thread_pool_st {
    event_loop_t main_loop;

    int thread_number;
    
    event_loop_thread_t event_loop_threads;
    
    int pos;
} *thread_pool_t;


thread_pool_t thread_pool_new(event_loop_t main_loop, int thread_number);


void thread_pool_start(thread_pool_t threadPool);

event_loop_t thread_pool_get_next_loop(thread_pool_t threadPool);


#ifdef __cplusplus
}
#endif

#endif