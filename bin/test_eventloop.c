#include "event_loop.h"
#include <stdio.h>

int main(int argc, char **argv) {
    event_loop_t loop = event_loop_init();
    printf("address: %p\n", loop);

    event_loop_wakeup(loop);
    event_loop_run(loop);
    return 0;
}