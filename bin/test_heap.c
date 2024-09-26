#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "heap.h"
#include "event.h"

#define container_of(ptr, type, member) \
((type*)((char*)(ptr) - offsetof(type, member)))

#define TIMER_ENTRY(p)          container_of(p, struct event_timer_st, node)

static int timers_compare(const struct heap_node* lhs, const struct heap_node* rhs) {
    return TIMER_ENTRY(lhs)->next_timeout < TIMER_ENTRY(rhs)->next_timeout;
}

// ((struct event_timer_st*)((char*)(timers->root) - __builtin_offsetof (struct event_timer_st, node)))

int main() {
    struct heap *timers = malloc(sizeof(struct heap));
    heap_init(timers, timers_compare);

    struct event_timer_st *timer1 = malloc(sizeof(struct event_timer_st));
    timer1->next_timeout = 10;

    struct event_timer_st *timer2 = malloc(sizeof(struct event_timer_st));
    timer2->next_timeout = 20;

    heap_insert(timers, &timer1->node);
    heap_insert(timers, &timer2->node);

    if(timers->root) {
        int64_t min_timeout = TIMER_ENTRY(timers->root)->next_timeout;
        printf("min timeout:%ld\n", min_timeout);
        heap_dequeue(timers);
        min_timeout = TIMER_ENTRY(timers->root)->next_timeout;
        printf("min timeout:%ld\n", min_timeout);
    }


    return 0;
}