#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include "event.h"

// CONN_EXPORT
// #if defined(HV_STATICLIB) || defined(HV_SOURCE)
//     #define CONN_EXPORT
// #elif defined(_MSC_VER)
//     #if defined(HV_DYNAMICLIB) || defined(CONN_EXPORTS) || defined(CONN_EXPORTS)
//         #define CONN_EXPORT  __declspec(dllexport)
//     #else
//         #define CONN_EXPORT  __declspec(dllimport)
//     #endif
// #elif defined(__GNUC__)
//     #define CONN_EXPORT  __attribute__((visibility("default")))
// #else
//     #define CONN_EXPORT
// #endif

typedef void (*event_cb) (event_t *ev);

typedef void (*io_cb) (io_t io);
typedef void (*read_cb) (io_t io, void *buf, int readybytes);
typedef void (*write_cb) (io_t io, const char *buf, int writebytes);
typedef void (*close_cb) (io_t io);
typedef void (*connect_cb) (io_t io);
typedef void (*accept_cb) (io_t io);

// event_loop interface
event_loop_t event_loop_init();

event_loop_t event_loop_init_with_name(const char *name);

int event_loop_destory(event_loop_t loop);

int event_loop_run(event_loop_t loop);

int event_loop_stop(event_loop_t loop);

int event_loop_wakeup(event_loop_t);

int io_add(io_t io, io_cb cb);
int io_remove(io_t io);

void io_set_readcb(io_t io, read_cb cb);
void io_set_writecb(io_t io, write_cb cb);
void io_set_closecb(io_t io, close_cb cb);
void io_set_connectcb(io_t io, connect_cb cb);
void io_set_acceptcb(io_t io, accept_cb cb);

void io_set_read_timeout(io_t io, int timeout);
void io_set_write_timeout(io_t io, int timeout);
void io_set_close_timeout(io_t io, int timeout);
void io_set_connect_timeout(io_t io, int timeout);


io_t create_tcp_client(event_loop_t loop, const char *ip, const char *port, connect_cb connect_cb, close_cb close_cb);

io_t create_tcp_server(event_loop_t loop, const char *ip, const char *port, accept_cb accept_cb);



#endif