#include <stdio.h>
#include "WebSocketServer.h"
#include "log.h"

class TestServer {
public:
    TestServer() {
        
    }

    ~TestServer() {
        
    }

    void onConnect() {
        LOG_I("ws server open");
    }

    void onClose() {
        LOG_I("ws client close");
    }

    void onMessage(const char *msg, int len, ws_opcode op_code) {
        // printf("on message: %s\n", msg);
        LOG_I("msg: %s", msg)
    }

};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }

    conn::WebSocketServer ws_server;
    conn::WebSocketService ws_service;
    ws_server.registerWebSocketService(&ws_service);


    //     std::function<void(const WebSocketChannel *, const HttpRequest *)>     onopen;
    // std::function<void(const WebSocketChannel *, const char *msg, size_t len, ws_opcode op_code)>   onmessage;
    // std::function<void(const WebSocketChannel *)>     onclose;
    ws_service.onopen = [](conn::WebSocketChannel * channel, const conn::HttpRequest * req) {
        TestServer *test_server = new TestServer();
        channel->setWSContext(test_server);
    };

    ws_service.onmessage = [](conn::WebSocketChannel * channel, const char *msg, size_t len, ws_opcode op_code) {
        TestServer *test_server = (TestServer *)channel->getWSContext();
        if (test_server) {
            test_server->onMessage(msg, len, op_code);
        }
        
        channel->send(msg, len, op_code);
    };

    ws_service.onclose = [](const conn::WebSocketChannel * channel) {
        TestServer *test_server = (TestServer *)channel->getWSContext();
        if (test_server) {
            test_server->onClose();
        }
        delete test_server;
    };
    
    // replace with your own cert and key fil
    // const char *cert_file = "/home/ubuntu/github/libconn/ssl/sslca/server.crt";
    // const char *key_file = "/home/ubuntu/github/libconn/ssl/sslca/server.key";
    int thread_num = 1;
    ws_server.run("127.0.0.1", argv[1], NULL, NULL, NULL, thread_num);
    return 0;
}