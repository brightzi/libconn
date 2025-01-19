#include "HttpServer.h"
#include "HttpHandle.h"
#include "event_loop.h"
#include <stdio.h>
#include <string.h>

namespace conn {

HttpServer::HttpServer() {
    m_router = nullptr;
    m_wsService = nullptr;
}

HttpServer::~HttpServer() {

}

void on_recv(io_t io, void *buf, int readbytes) {
    HttpHandle *handle = (HttpHandle *)io->userdata;
    // printf("on_recv: %s\n", buf);
    if (handle->feedRecvData((char *)buf, readbytes) != readbytes) {
        io_close(io);
    }
}

void on_close(io_t io) {
    printf("on close\n");
    HttpHandle *handle = (HttpHandle *)io->userdata;
    delete handle;
}

void on_accept(io_t io) {
    HttpServer *server = (HttpServer *)io->userdata;

    HttpHandle *handle = new HttpHandle(io);
    handle->setRouter(server->getRouter(), server->getWSService());
    io->userdata = handle;
    io_set_readcb(io, on_recv);
    io_set_closecb(io, on_close);
}

void HttpServer::registerHttpRouter(HttpRouter *router) {
    m_router = router;
}

HttpRouter* HttpServer::getRouter() {
    return m_router;
}

WebSocketService *HttpServer::getWSService() {
    return m_wsService;
}

void HttpServer::run(const char *ip, const char *http_port, const char *https_port) {
    event_loop_t loop = event_loop_init();
    if (http_port) {
        io_t io = create_tcp_server(loop, ip, http_port, on_accept);
        if (io == NULL) {
            return ;
        }
        io->userdata = this;
    }
    if (https_port) {
        io_t io = create_ssl_server(loop, ip, https_port, on_accept);
        if (io == NULL) {
            return ;
        }
        io->userdata = this;
    }
    event_loop_run(loop);
}



}