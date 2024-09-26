#include "event.h"

void eventcb(event_t *ev) {
    printf("event_cb called\n");
    return ;
}

int main(int argc, char *argv[]) {
    io_t io = malloc(sizeof(io_t));
    if (!io) {
        return -1;
    }
    return 0;
}