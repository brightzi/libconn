#ifndef IO_H
#define IO_H
#include "event.h"

int create_socket(int block);
void close_socket(io_t io);

void handle_event(io_t io);

int io_connect(io_t io);
int io_add(io_t io, event_cb cb, int event);
int io_del(io_t, int event);
int io_write(io_t io, const void *buf, size_t len);

#endif