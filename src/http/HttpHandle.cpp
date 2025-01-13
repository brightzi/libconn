#include "HttpHandle.h"
#include "event_loop.h"

namespace conn {

HttpHandle::HttpHandle(io_t io) : m_router(NULL),
m_parser(NULL), m_request(NULL), m_response(NULL), m_io(io){

}

HttpHandle::~HttpHandle() {
    if (m_parser) {
        delete m_parser;
        m_parser = NULL;
    }

    if (m_request) {
        delete m_request;
        m_request = NULL;
    }

    if (m_response) {
        delete m_response;
        m_response = NULL;
    }
}

int HttpHandle::init(HttpRouter *router) {
    if (router == NULL) {
        return -1;
    }

    m_router = router;

    if (m_parser == NULL) {
        m_parser = new HttpParser(HTTP_PARSER_REQUEST);
    }
    m_request = new HttpRequest();
    m_response = new HttpResponse();
    m_parser->initHttpRequest(m_request);
    m_parser->initHttpResponse(m_response);
    return 0;
}

int HttpHandle::feedRecvData(const char *data, int len) {   
    int nparse = m_parser->feedRecvData(data, len);
    if (nparse != len) {
        return -1;
    }

    if (m_parser->isComplete()) {
        sendHttpResponse();
    }

    return len; 
}

int HttpHandle::invokeHttpFunc() {
    const handle_func *func = m_router->getHandleFunc(m_parser->method, m_parser->url.c_str());
    if (!func) {
        return -1;
    }
    (*func)(m_request, m_response);
    return 0;
}

void HttpHandle::sendHttpResponse() {
    if (invokeHttpFunc() != 0) {
        return ;
    }

    const std::string &data = m_response->dump();
    io_send_data(m_io, data.c_str(), data.size());
}


};