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


int main() {
    heap_t timers = create_heap(20, timers_compare);

    for (int i = 0; i < 10; i++) {
        struct event_timer_st *timer = malloc(sizeof(struct event_timer_st));
        timer->next_timeout = i;
        heap_insert(timers, &timer->node);
    }

    for (int i = 0; i < 20; i++) {
        heap_node_t node = heap_top(timers);
        if (node == NULL) {
            break;
        }
        struct event_timer_st *timer =  TIMER_ENTRY(node);
        printf("pop: %ld\n", timer->next_timeout);
        heap_pop(timers);
    }

    // struct event_timer_st *timer1 = malloc(sizeof(struct event_timer_st));
    // timer1->next_timeout = 10;

    // struct event_timer_st *timer2 = malloc(sizeof(struct event_timer_st));
    // timer2->next_timeout = 20;

    // struct event_timer_st *timer3 = malloc(sizeof(struct event_timer_st));
    // timer3->next_timeout = 30;

    // heap_insert(timers, &timer1->node);
    // heap_insert(timers, &timer2->node);
    // heap_insert(timers, &timer3->node);
    

    // if(timers->array) {
    //     int64_t min_timeout = TIMER_ENTRY(timers->array[0])->next_timeout;
    //     printf("min timeout:%ld\n", min_timeout);
    //     printf("before pop size: %d", timers->size);
    //     heap_pop(timers);
    //     printf("aftre pop size: %d", timers->size);


    //     min_timeout = TIMER_ENTRY(timers->array[0])->next_timeout;
    //     printf("min timeout:%ld\n", min_timeout);
    //     printf("before pop size: %d", timers->size);
    //     heap_pop(timers);
    //     printf("aftre pop size: %d", timers->size);

    //      min_timeout = TIMER_ENTRY(timers->array[0])->next_timeout;
    //     printf("min timeout:%ld\n", min_timeout);
    //     printf("before pop size: %d", timers->size);
    //     heap_pop(timers);
    //     printf("aftre pop size: %d", timers->size);
    // }


    return 0;
}