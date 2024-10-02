#include "event_loop.h"
#include <stdio.h>


void on_read(io_t io, void *buf, int readybytes) {
    printf("on message\n"); 
    char temp[1024];
    read(io->fd, temp, 1024);
    printf("recv: %s\n", temp);
    sleep(1);  
    write(io->fd, "hello, world!", strlen("hello, world!"));
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

}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return -1;
    }
    event_loop_t loop = event_loop_init();
    io_t io = create_tcp_client(loop, argv[1], argv[2], on_connect, on_close);
    io_set_readcb(io, on_read);
    
    event_loop_run(loop);
    return 0;
}