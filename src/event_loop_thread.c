#include "event_loop_thread.h"

#include <stdio.h>

int event_loop_thread_init(event_loop_thread_t event_loop_thread, int index) {
    if (event_loop_thread == NULL) {
        return -1;
    }
    pthread_mutex_init(&event_loop_thread->mutex, NULL);
    pthread_cond_init(&event_loop_thread->cond, NULL);
    event_loop_thread->eventLoop = NULL; 
    event_loop_thread->tid = 0;
    event_loop_thread->thread_count = 0;
    
    char *name = calloc(1, 32 * sizeof(char));
    if (name == NULL) {
        return -1;
    }
    sprintf(name, "Thread:%d", index);
    event_loop_thread->thread_name = name;
    return 0;
}

void *event_loop_thread_run(void *arg) {
    event_loop_thread_t event_loop_thread = (event_loop_thread_t) arg;
    pthread_mutex_lock(&event_loop_thread->mutex);
    event_loop_thread->eventLoop = event_loop_init_with_name(event_loop_thread->thread_name);
    printf("eventLoop:%p\n", event_loop_thread->eventLoop);
    pthread_cond_signal(&event_loop_thread->cond);
    pthread_mutex_unlock(&event_loop_thread->mutex);
    
    event_loop_run(event_loop_thread->eventLoop);
}


void event_loop_thread_start(event_loop_thread_t event_loop_thread) {
    pthread_create(&event_loop_thread->tid, NULL, &event_loop_thread_run, event_loop_thread);
    pthread_mutex_lock(&event_loop_thread->mutex);

    while(event_loop_thread->eventLoop == NULL) {
        pthread_cond_wait(&event_loop_thread->cond, &event_loop_thread->mutex);
    }
    pthread_mutex_unlock(&event_loop_thread->mutex);
    return ;
}