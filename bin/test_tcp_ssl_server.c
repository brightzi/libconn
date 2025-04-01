#include "event_loop.h"
#include <stdio.h>

void on_read(io_t io, char *buf, int readybytes) {
    printf("on read: %s\n", buf);
    io_send_data(io, "nihao", 5);
}

void on_accept(io_t io) {
    printf("on accept\n");
    io_set_readcb(io, on_read);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: %s ip port cert_file key_file\n", argv[0]);
        return -1;
    }
    event_loop_t loop = event_loop_init();
    const char *cert_file = "/home/ubuntu/github/libconn/ssl/sslca/server.crt";
    const char *key_file = "/home/ubuntu/github/libconn/ssl/sslca/server.key";
    int thread_num = 4;
    io_t io = create_ssl_server(loop, argv[1], argv[2], cert_file, key_file, on_accept, thread_num);
    event_loop_run(loop);
    return 0;
}