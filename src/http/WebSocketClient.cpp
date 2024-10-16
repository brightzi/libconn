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
    m_wsParser = std::make_shared<WebSocketParser>();
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
                m_wsParser->onMessage = [this](int opcode, const std::string &msg) {
                    if (onmessage) {
                        onmessage(msg.c_str(), msg.size());
                    }
                };
            }
        }

        if (m_wsState == WS_STATE_OPENED && size > 0) {
            // TODO: parse websocket frame
            // printf("ws onread:%s\n", data);
            m_wsParser->feedRecvData(data, size);
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
    if (m_wsState != WS_STATE_OPENED) {
        return ;
    }
    int send_len = 0;
    const char *data = m_wsParser->buildFrame(msg, len, WS_OPCODE_TEXT, send_len);
    if (data == NULL || send_len == 0) {
        return ;
    }

    m_channel->sendData(data, send_len);
}

void WebSocketClient::close() {
    m_channel->close();
    m_wsState = WS_STATE_CLOSED;
}

bool WebSocketClient::isConnected() {
    return m_wsState == WS_STATE_OPENED;
}

static void timer_callback(event_timer_t timer) {
    printf("timer_trigger\n");
    WebSocketClient *client = (WebSocketClient *)timer->privdata;
    client->close();
    
    sleep(1);
    std::map<std::string, std::string> headers;
    // ws->open("http://127.0.0.1:8888", headers);
    client->open("http://124.222.224.186:8800", headers);
    client->closeAfterTime(3000);
}

void WebSocketClient::closeAfterTime(int time_ms) {
    event_timer_t timer = add_timer(m_loop_thread->getLoop()->loop(), 3000, timer_callback, 0);
    timer->privdata = this;
}

}
