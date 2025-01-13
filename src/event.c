#include "event.h"

void event_pending(event_t ev) {
    if (!ev->pending) {
        ev->pending = 1;
        event_t *pHead = &(ev->loop->pending);
        ev->next_event = *pHead;
        *pHead = ev;
    }
    return ;
}

void event_del(event_t ev) {
    if (ev->destory) {
        printf("del:%p\n", ev);
        ev->destory = 0;
        free(ev);
    }
}

void event_reset(event_t ev) {
    ev->destory = 0;
    ev->active = 1;
    ev->pending = 0;
}