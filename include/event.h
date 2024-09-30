#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>
#include "dispatcher.h"
#include "heap.h"

typedef struct event_st event_st, *event_t;
typedef struct event_loop_st event_loop_st, *event_loop_t;
typedef struct event_timer_st event_timer_st, *event_timer_t;
typedef struct io_st io_st, *io_t;

typedef void (*event_cb) (event_t *ev);

typedef void (*io_cb) (io_t io);
typedef void (*read_cb) (io_t io, void *buf, int readybytes);
typedef void (*write_cb) (io_t io, const char *buf, int writebytes);
typedef void (*close_cb) (io_t io);
typedef void (*connect_cb) (io_t io);
typedef void (*accept_cb) (io_t io);

#define EVENT_READ  0x01
#define EVENT_WRITE 0x02
#define EVENT_RW    0x03

typedef enum event_type{
    event_io,
    event_timer,
    event_signal,
} event_type;

#define EVENT_FILEDS \
    event_loop_t loop; \ 
    event_type etype; \ 
    uint64_t event_id;\ 
    event_cb cb; \ 
    void *userdata; \ 
    event_t next_event; \
    unsigned pending;

struct event_st {
    EVENT_FILEDS
};

struct event_timer_st {
    EVENT_FILEDS
    uint32_t repeat;
    uint64_t next_timeout;
    struct heap_node node;
};

struct io_st {
    EVENT_FILEDS
    int fd;
    int events;
    int revents;
    void *iodata;

    uint64_t last_read_time;
    uint64_t last_write_time;

    int connect_timeout; 
    int close_timeout;
    int read_timeout;
    int write_timeout;
    int heartbeat_interval;

    char *ip;
    char *port;
    struct sockaddr_in *local_addr;
    struct sockaddr_in *peer_addr;

    io_cb io_cb;

    read_cb read_cb;
    write_cb write_cb;
    close_cb close_cb;
    connect_cb connect_cb;
    accept_cb accept_cb;

    event_timer_t read_timer;
    event_timer_t write_timer;
    event_timer_t close_timer;
    event_timer_t connect_timer;
};

struct event_loop_st {
    int quit;
    char name[32];
    int tid;
    uint64_t start_ms;
    uint64_t cur_ms;

    io_st **io_array;  // fd -> event_io_t
    uint32_t io_num;
    uint32_t io_maxsize;

    struct heap timers;
    uint32_t timers_num;

    event_t pending;  
    uint32_t npendings;

    int pipefd[2]; // pipe[0] for read, pipe[1] for write
    const struct event_dispatcher *disp;
    void *disp_data;
};

void event_pending(event_t ev);


#endif