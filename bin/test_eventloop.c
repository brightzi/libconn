#include "event_loop.h"
#include <stdio.h>

int main(int argc, char **argv) {
    event_loop_t loop = event_loop_init();
    printf("address: %p", loop);
    return 0;
}