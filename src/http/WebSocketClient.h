#ifndef WEB_SOCKET_CLIENT_H
#define WEB_SOCKET_CLIENT_H
#include "HttpMessage.h"
#include "HttpParser.h"
#include "EventLoopThread.h"
#include "Channel.h"

namespace conn {

typedef enum ws_opcode {
    WS_OPCODE_CONTINUE = 0x0,
    WS_OPCODE_TEXT     = 0x1,
    WS_OPCODE_BINARY   = 0x2,
    WS_OPCODE_CLOSE    = 0x8,
    WS_OPCODE_PING     = 0x9,
    WS_OPCODE_PONG     = 0xA
} ws_opcode;

typedef enum ws_state {
    WS_STATE_CONNECTING = 0,
    WS_STATE_CONNECTED,
    WS_STATE_UPGRADING,
    WS_STATE_OPENED,
    WS_STATE_CLOSED
} ws_state;

class WebSocketClient {
public:
    WebSocketClient(EventLoopThread *loop_thread = NULL);
    ~WebSocketClient();

    std::function<void()>   onopen;
    std::function<void(const char *msg, int len)> onmessage;
    std::function<void()>        onclose;

    int open(const char *url, const http_headers & headers);

    void send(const char *msg, int len, ws_opcode opcode);

    void close();

private:
    int sendHttpRequest();

private:
    HttpParserPtr m_httpParser; 
    HttpRequestPtr m_httpRequest;
    HttpResponsePtr m_httpReponse;
    ChannelPtr m_channel;
    EventLoopThread *m_loop_thread;
    ws_state m_wsState;
};

}

#endif