#include "WebSocketClient.h"
#include "io.h"
#include "base64.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <functional> 

namespace conn {

WebSocketClient::WebSocketClient(EventLoopThread *loop_thread) :
    m_loop_thread(loop_thread){

}

WebSocketClient::~WebSocketClient() {

}

int WebSocketClient::sendHttpRequest() {

    // std::string str = "GET / HTTP/1.1\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: St/9ZjRDT6QAmTaAuNIi8Q==\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\nHost: 127.0.0.1:8888\r\n\r\n";
    // m_channel->sendData(str.c_str(), str.length());

    m_httpRequest->headers["Connection"] = "Upgrade";
    m_httpRequest->headers["Upgrade"] = "websocket";

    unsigned char rand_key[16] = {0};
    int *p = (int*)rand_key;
    for (int i = 0; i < 4; ++i, ++p) {
        *p = rand();
    }
    char ws_key[32] = {0};
    base64_encode(rand_key, 16, ws_key);
    m_httpRequest->headers["Sec-WebSocket-Key"] = ws_key;
    m_httpRequest->headers["Sec-WebSocket-Version"] = "13";

    std::string str;
    str.append("GET / HTTP/1.1\r\n");
    for (const auto &it : m_httpRequest->headers) {
        str.append(it.first).append(": ").append(it.second).append("\r\n");
    }
    str.append("\r\n");
    m_httpParser = std::make_shared<HttpParser>();
    m_httpReponse = std::make_shared<HttpResponse>();
    m_httpParser->initHttpResponse(m_httpReponse.get());
    m_channel->sendData(str.c_str(), str.length());
    m_wsState = WS_STATE_UPGRADING;
    return 0;
}

int WebSocketClient::open(const char *url, const http_headers & headers) {
    if (m_loop_thread == NULL) {
        m_loop_thread = new EventLoopThread();
        m_loop_thread->init();
        usleep(10000);
    }

    m_httpRequest = std::make_shared<HttpRequest>();
    m_httpRequest->url = url;
    m_httpRequest->parseUrl();

    int block = 0;
    int client_fd = create_socket(block);
    io_t io = get_io(m_loop_thread->getLoop()->loop(), client_fd);
    if (io == NULL) {
        return -1;
    }
    io->ip = strdup(m_httpRequest->u->host);
    char port[16] = {0};
    snprintf(port, sizeof(port), "%d", m_httpRequest->u->port);
    io->port = strdup(port);
    m_channel = std::make_shared<Channel>(io); 
    m_channel->init();
    io->ctx = m_channel.get();

    m_channel->onconnect = [this]() {
        // printf("tcp onconnect\n");
        m_wsState = WS_STATE_CONNECTED;
        sendHttpRequest();
    };

    m_channel->onread = [this](Buffer * buf) {
        // printf("tcp onread:%s\n", buf->data());
        int size = buf->size();

        const char *data = (const char *)buf->data();
        if (m_wsState == WS_STATE_UPGRADING) {
            int nparse = m_httpParser->feedRecvData(data, size);
            if (nparse != size && m_httpParser->getError()) {
                m_channel->close();
                return ;
            }
            data += nparse;
            size -= nparse;

            if (m_httpParser->isComplete()) {
                if (m_httpReponse->status_code != 101) {
                    return m_channel->close();
                }
            }

            m_wsState = WS_STATE_OPENED;
            if (onopen) {
                onopen();
            }
        }

        if (m_wsState == WS_STATE_OPENED && size > 0) {
            // TODO: parse websocket frame
            // printf("ws onread:%s\n", data);
            if (onmessage) {
                onmessage(data, size);
            }
        }
    };

    m_channel->onclose = [this]() {
        if (onclose) {
            onclose();
        }
    };

    m_wsState = WS_STATE_CONNECTING;
    m_channel->startConnect();
    return 0;
}

void WebSocketClient::send(const char *msg, int len, ws_opcode opcode) {


}

void WebSocketClient::close() {

}

}
