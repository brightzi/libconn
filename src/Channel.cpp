#include "Channel.h"
#include "io.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

namespace conn {

Channel::Channel(io_t io) {
    m_io = io;
    m_readBuffer = NULL;
    m_ctx = NULL;
    // m_writeBuffer = NULL;
}

Channel::~Channel() {
    printf("channel delete\n");
}

static void on_read(io_t io, void* data, int readbytes) {
    Channel* channel = (Channel*)io->ctx;
    if (channel && channel->onread) {
        channel->getReadBuff()->setdata(data, readbytes);
        channel->onread(channel->getReadBuff());
    }
}

static void on_write(io_t io, const char* data, int writebytes) {
    Channel* channel = (Channel *)io->ctx;
    if (channel && channel->onwrite) {
        // channel->onwrite(&buf);
    }
}

static void on_close(io_t io) {
    Channel *channel = (Channel*)io->ctx;
    if (channel && channel->onclose) {
        channel->onclose();
    }
}

static void on_connect(io_t io) {
    Channel *channel = (Channel*)io->ctx;
    if (channel && channel->onconnect) {
        channel->onconnect();
    }

}

int Channel::init() {
    m_readBuffer = new Buffer();
    // m_writeBuffer = new Buffer();
    io_set_readcb(m_io, on_read);
    io_set_closecb(m_io, on_close);
    io_set_writecb(m_io, on_write);
    io_set_connectcb(m_io, on_connect);
    return 0;
}

void Channel::startConnect() {
    io_connect(m_io);
}

void Channel::sendData(const char *buf, int len) {
    io_write(m_io, buf, len);
}

void Channel::close() {
    io_close(m_io);
}

}