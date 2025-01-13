#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include "HttpRouter.h"
#include <functional>
#include <memory>

namespace conn { 
class HttpServer {
public:
    HttpServer();
    virtual ~HttpServer();

    void registerHttpRouter(HttpRouter *router);

    void run(const char *ip, const char * http_port, const char *https_port);

    HttpRouter* getRouter();

private:
    HttpRouter *m_router; 

};

}

#endif