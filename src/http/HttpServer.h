#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include "HttpRouter.h"
#include <functional>
#include <memory>

namespace conn { 
class WebSocketService;
class HttpServer {
public:
    HttpServer();
    virtual ~HttpServer();

    void registerHttpRouter(HttpRouter *router);

    void run(const char *ip, const char * http_port, const char *https_port, const char *cert_file, const char *key_file, int thread_num);

    HttpRouter* getRouter();
    WebSocketService *getWSService();

    void setWebSocketService(WebSocketService *ws_service) {
        m_wsService = ws_service;
    }

private:
    HttpRouter *m_router; 
    WebSocketService * m_wsService;

};

}

#endif