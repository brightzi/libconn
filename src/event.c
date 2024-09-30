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