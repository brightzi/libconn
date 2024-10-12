#include "event_loop.h"
#include <sys/time.h>
#include "ctimer.h"

long before_time = 0;
long after_time = 0;

void timer_callback(event_timer_t timer, void *arg) {
    after_time = get_curtime_ms();
    printf("Timer expired, %d\n", after_time - before_time);
}

int main(int argc, char **argv) {

    event_loop_t loop = event_loop_init();

    add_timer(loop, 5000, timer_callback, 0);
    add_timer(loop, 10000, timer_callback, 0);
    add_timer(loop, 20000, timer_callback, 0);
    before_time = get_curtime_ms();

    event_loop_run(loop);
    return 0;
}