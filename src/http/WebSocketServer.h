#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H
#include "HttpServer.h"
#include "WebSocketChannel.h"

namespace conn {

struct WebSocketService {
    std::function<void(WebSocketChannel *, const HttpRequest *)>     onopen;
    std::function<void(const WebSocketChannel *, const char *msg, size_t len, ws_opcode op_code)>   onmessage;
    std::function<void(const WebSocketChannel *)>     onclose;
    int ping_interval;

    WebSocketService() : ping_interval(0) {}

    void setPingInterval(int ms) {
        ping_interval = ms;
    }
};



class WebSocketServer : public HttpServer{
public:
    WebSocketServer() {

    }

    virtual ~WebSocketServer() {

    }

    void registerWebSocketService(WebSocketService *service) {
        setWebSocketService(service);
    }

private:  

private:
};

}


#endif