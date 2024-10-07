#ifndef CHANNEL_H
#define CHANNEL_H
#include "event_loop.h"
#include "Buffer.h"
#include <functional>

namespace conn {

class Channel {
public:
    std::function<void()>   onconnect;
    std::function<void(Buffer*)> onread;
    std::function<void(Buffer*)> onwrite;
    std::function<void()>        onclose;

    Channel(io_t io);
    ~Channel();
    int init();
    void write();    

    void startConnect();
    void sendData(const char *buf, int len);
    void close();

    void *m_ctx;

    Buffer *getReadBuff() {
        return m_readBuffer;
    }

private:
    Buffer *m_readBuffer;
    // Buffer *m_writeBuffer;
    io_t m_io;
};

}

#endif