#ifndef IO_H
#define IO_H
#include "event.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void make_noblock_fd(int fd);

int create_socket();
void close_socket(io_t io);

int  io_connect(io_t io);
void io_add(io_t io, io_cb cb, int event);
void io_del(io_t, int event);

#endif