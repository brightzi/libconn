#ifndef IO_H
#define IO_H
#include "event.h"
#ifdef __cplusplus
extern "C" {
#endif 
int create_socket(int block);
void make_noblock_fd(int fd);
void close_socket(io_t io);

void handle_event(io_t io);

int io_connect(io_t io);
int io_add(io_t io, event_cb cb, int event);
int io_del(io_t, int event);
int io_write(io_t io, const void *buf, size_t len);
int io_set_reuse_addr(io_t io);
int io_bind(io_t io);
int io_listen(io_t io);
int io_accept(io_t io);

void io_set_read_timeout(io_t io, int timeout);
void io_set_write_timeout(io_t io, int timeout);
void io_set_close_timeout(io_t io, int timeout);
void io_set_connect_timeout(io_t io, int timeout);


void io_remove_read_timeout(io_t io);
void io_remove_write_timeout(io_t io);



#ifdef __cplusplus
}
#endif
#endif