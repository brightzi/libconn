#include "thread_pool.h""

thread_pool_t thread_pool_new(event_loop_t main_loop, int thread_number) {
    thread_pool_t threadPool = malloc(sizeof(struct thread_pool_st));
    if (threadPool == NULL) {
        return NULL;
    }
    
    threadPool->main_loop = main_loop;
    threadPool->pos = 0;
    threadPool->thread_number = thread_number;
    threadPool->event_loop_threads = NULL;

    return threadPool; 
}


void thread_pool_start(thread_pool_t threadPool) {
    if (threadPool->thread_number <= 0) {
        return ;
    }
    
    threadPool->event_loop_threads = malloc(threadPool->thread_number * sizeof(struct event_loop_thread_st));
    for (int i = 0; i < threadPool->thread_number; i++) {
        event_loop_thread_init(&threadPool->event_loop_threads[i], i);
        event_loop_thread_start(&threadPool->event_loop_threads[i]);
    }
    
    return ;
}

event_loop_t thread_pool_get_next_loop(thread_pool_t threadPool) {
    struct event_loop_st *loop = threadPool->main_loop;
    
    if (threadPool->thread_number > 0) {
        loop = threadPool->event_loop_threads[threadPool->pos].eventLoop;
        threadPool->event_loop_threads[threadPool->pos].thread_count++;
        threadPool->pos = (++threadPool->pos) % threadPool->thread_number;
    }
    return loop;
}