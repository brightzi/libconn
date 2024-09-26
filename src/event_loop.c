#include "event_loop.h"

event_loop_t event_loop_init() {
    return event_loop_init_with_name(NULL);
}

event_loop_t event_loop_init_with_name(const char *name) {
    event_loop_t loop = malloc(sizeof(event_loop_st));
    if (loop == NULL) {
        return NULL;
    }
    loop->start_ms = get_curtime_ms();
    loop->cur_ms = get_curtime_ms();


    return loop;    
}

int event_loop_destory(event_loop_t loop);

int event_loop_run(event_loop_t loop);

int event_loop_stop(event_loop_t loop);

int event_loop_wakeup(event_loop_t);