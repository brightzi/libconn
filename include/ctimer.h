#ifndef TIMER_H
#define TIMER_H
#include <sys/time.h>
#include <stddef.h>

#define sleep_ms(ms) usleep(ms * 1000)

static long get_curtime_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000+ tv.tv_usec / 1000;
}

static long get_monotime_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}


#endif