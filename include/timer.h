#ifndef TIMER_H
#define TIMER_H
#include <sys/time.h>
#include <stddef.h>

#define sleep_ms(ms) usleep(ms * 1000)

static long long get_curtime_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000+ tv.tv_usec / 1000;
}


#endif