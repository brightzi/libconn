#ifndef WEBSOCKET_CHANNEL_H
#define WEBSOCKET_CHANNEL_H
#include <mutex>
#include "Channel.h"
#include "WebSocketParser.h"


namespace conn {

class WebSocketChannel : public Channel {

public:
    WebSocketChannel(io_t io, ws_session_type type);
    virtual ~WebSocketChannel();

    int send(const std::string&msg);

    int send(const char *buf, int len, ws_opcode opcode = WS_OPCODE_BINARY);

    int sendPing();

    int sendPong();

    ws_opcode opcode;

    void setWSContext(void *wsContext) {
        m_wsContext = wsContext;
    }

    void *getWSContext() const {
        return m_wsContext;
    }

private:
    int sendFrame(const char *data, int len, ws_opcode opcode, bool fin);

private:
    void *m_wsContext;

    std::mutex m_mutex;
    ws_session_type m_type;
    char *m_buf;
    int m_bufLen;
};
}

typedef std::shared_ptr<conn::WebSocketChannel> WebSocketChannelPtr;

#endif