#include "event_loop.h"
#include <stdio.h>

void on_close(io_t io) {
    printf("on close\n");
}

void on_read(io_t io, char *buf, int readybytes) {
    printf("on read: %s\n", buf);
    io_send_data(io, "nihao", 5);
}

void on_accept(io_t io) {
    printf("on accept\n");
    io_set_readcb(io, on_read);
    io_set_closecb(io, on_close);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s ip port", argv[0]);
        return -1;
    }
    event_loop_t loop = event_loop_init();
    io_t io = create_tcp_server(loop, argv[1], argv[2], on_accept);
    event_loop_run(loop);
    return 0;
}