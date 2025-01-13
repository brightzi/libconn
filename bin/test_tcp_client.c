#include "event_loop.h"
#include <stdio.h>

static  char *ip = NULL;
static  char *port = NULL;

void on_read(io_t io, void *buf, int readybytes) {
    printf("on message\n"); 
    // read(io->fd, temp, 1024);
    printf("recv: %s\n", buf);
    sleep(1);  
    write(io->fd, "hello, world!", strlen("hello, world!"));
    io_write(io, "hello, world!", strlen("hello, world!"));
}


void on_connect(io_t io) {
    printf("on connect\n");
    io_send_data(io, "hello, world", strlen("hello, world"));
    io_set_readcb(io, on_read);     
    io_read_enable(io);
    return ;
}

void on_close(io_t io) {
    printf("on close\n");
    sleep(2);
    new_connect(io->loop);
}

void new_connect(event_loop_t loop) {
    printf("new connect\n");
    io_t io = create_tcp_client(loop, ip, port, on_connect, on_close, NULL);
    if (io == NULL) {
        return -1;
    }
    io_set_readcb(io, on_read);
    io_set_read_timeout(io, 20000);
    io_set_write_timeout(io, 5000);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return -1;
    }
    event_loop_t loop = event_loop_init();
    ip = strdup(argv[1]);
    port = strdup(argv[2]);
    new_connect(loop);

    event_loop_run(loop);
    return 0;
}