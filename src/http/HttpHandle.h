#ifndef HTTP_HANDLE_H
#define HTTP_HANDLE_H
#include "HttpRouter.h"
#include "HttpParser.h"
#include "WebSocketParser.h"
#include "WebSocketServer.h"
#include "WebSocketChannel.h"
#include "event.h"

typedef enum {
    SERVICE_HTTP = 0,
    SERVICE_WEBSOCKET
} SERVICE_TYPE;

#define HTTP_HEADER "HTTP/1.1 200 OK\r\n"
#define WS_HEADER  "HTTP/1.1 101 Switching Protocols\r\n"
namespace conn {

class HttpHandle {
public:
    HttpHandle(io_t io);
    virtual ~HttpHandle();

    int setRouter(HttpRouter *router, WebSocketService* ws_service);

    int feedRecvData(const  char *data, int len);

private:
    int invokeHttpFunc();
    void sendHttpResponse();
    bool upgradeWSProtocol();
    bool switchWSProtocol();
    bool isUpgradeWSProtocol();
    void close();

    HttpRouter *m_router;
    HttpParser *m_parser;
    HttpRequest *m_request;
    HttpResponse *m_response;
    WebSocketParser *m_wsParser;
    WebSocketService *m_wsService;
    WebSocketChannel *m_wsChannel;
    SERVICE_TYPE m_serviceType;
    io_t m_io; 

};

};

#endif