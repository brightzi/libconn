#ifndef HTTP_HANDLE_H
#define HTTP_HANDLE_H
#include "HttpRouter.h"
#include "HttpParser.h"
#include "event.h"

namespace conn {

class HttpHandle {
public:
    HttpHandle(io_t io);
    virtual ~HttpHandle();

    int init(HttpRouter *router);

    int feedRecvData(const  char *data, int len);

private:
    int invokeHttpFunc();
    void sendHttpResponse();

    HttpRouter *m_router;
    HttpParser *m_parser;
    HttpRequest *m_request;
    HttpResponse *m_response;
    io_t m_io; 
};

};

#endif