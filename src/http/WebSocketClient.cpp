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
    m_loop_thread(loop_thread),
    m_httpParser(nullptr),
    m_httpRequest(nullptr), 
    m_httpReponse(nullptr),
    m_wsParser(nullptr),
    m_channel(nullptr),
    m_wsState(WS_STATE_CLOSED),
    m_pingInterVal(0),
    m_pingTimer(nullptr) {
    // 初始化随机数种子
    srand(time(NULL));
}

WebSocketClient::~WebSocketClient() {
    // 析构时需要清理资源
    if (m_channel) {
        m_channel->close();
    }
    if (m_loop_thread) {
        delete m_loop_thread;
        m_loop_thread = nullptr;
    }
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
    // 这里应该使用实际的path而不是硬编码的"/"
    str.append("GET /").append(m_httpRequest->u->path).append(" HTTP/1.1\r\n");
    // 添加Host头
    str.append("Host: ").append(m_httpRequest->u->host);
    if (m_httpRequest->u->port != 80) {
        str.append(":").append(std::to_string(m_httpRequest->u->port));
    }
    str.append("\r\n");
    for (const auto &it : m_httpRequest->headers) {
        str.append(it.first).append(": ").append(it.second).append("\r\n");
    }
    str.append("\r\n");
    m_httpParser = std::make_shared<HttpParser>(HTTP_PARSER_RESPONSE);
    m_httpReponse = std::make_shared<HttpResponse>();
    m_wsParser = std::make_shared<WebSocketParser>();
    m_httpParser->initHttpResponse(m_httpReponse.get());
    m_channel->sendData(str.c_str(), str.length());
    m_wsState = WS_STATE_UPGRADING;
    return 0;
}

int WebSocketClient::open(const char *url, const http_headers & headers) {
    if (m_wsState != WS_STATE_CLOSED) {
        return -1; // 已经打开或正在连接中
    }

    if (m_loop_thread == NULL) {
        m_loop_thread = new EventLoopThread();
        m_loop_thread->init();
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
    if (strcasecmp(m_httpRequest->u->scheme, "ws") == 0){
        io->type = IO_TYPE_TCP;
    } else if (strcasecmp(m_httpRequest->u->scheme, "wss") == 0) {
        io->type = IO_TYPE_SSL;
    } else {
        return -1; // 不支持的协议
    }

    m_channel->onconnect = [this]() {
        printf("tcp onconnect\n");
        m_wsState = WS_STATE_CONNECTED;
        sendHttpRequest();
    };

    m_channel->onread = [this](Buffer * buf) {
        int size = buf->size();

        const char *data = (const char *)buf->data();
        if (m_wsState == WS_STATE_UPGRADING) {
            int nparse = m_httpParser->feedRecvData(data, size);
            if (nparse != size && m_httpParser->getError()) {
                printf("http parse error\n");
                m_channel->close();
                return ;
            }
            data += nparse;
            size -= nparse;

            if (m_httpParser->isComplete()) {
                if (m_httpReponse->status_code != 101) {
                    printf("websocket upgrade failed, status code: %d\n", m_httpReponse->status_code);
                    return m_channel->close();
                }
                
                // 验证服务器返回的Sec-WebSocket-Accept
                auto it = m_httpReponse->headers.find("Sec-WebSocket-Accept");
                if (it == m_httpReponse->headers.end()) {
                    printf("missing Sec-WebSocket-Accept header\n");
                    return m_channel->close();
                }

                m_wsState = WS_STATE_OPENED;
                if (onopen) {
                    onopen();
                }
                m_wsParser->onMessage = [this](int opcode, const std::string &msg) {
                    if (onmessage) {
                        onmessage(msg.c_str(), msg.size());
                    }
                };

                // 如果设置了ping间隔,开始定时发送ping
                if (m_pingInterVal > 0) {
                    m_pingTimer = add_timer(m_loop_thread->getLoop()->loop(), m_pingInterVal, [](event_timer_t timer) {
                        WebSocketClient *client = (WebSocketClient *)timer->privdata;
                        if (client->isConnected()) {
                            client->send("", 0, WS_OPCODE_PING);
                        }
                    }, UINT32_MAX);
                    m_pingTimer->privdata = this;
                }
            }
        }

        if (m_wsState == WS_STATE_OPENED && size > 0) {
            m_wsParser->feedRecvData(data, size);
        }
    };

    m_channel->onclose = [this]() {
        printf("tcp onclose\n");
        m_wsState = WS_STATE_CLOSED;
        if (m_pingTimer) {
            del_timer(m_loop_thread->getLoop()->loop(), m_pingTimer);
            m_pingTimer = nullptr;
        }
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
    // 使用传入的opcode而不是硬编码WS_OPCODE_TEXT
    const char *data = m_wsParser->buildFrame(msg, len, opcode, send_len);
    if (data == NULL || send_len == 0) {
        return ;
    }

    m_channel->sendData(data, send_len);
}

void WebSocketClient::close() {
    if (m_channel) {
        m_channel->close();
    }
    m_wsState = WS_STATE_CLOSED;
}

bool WebSocketClient::isConnected() {
    return m_wsState == WS_STATE_OPENED;
}

static void timer_callback(event_timer_t timer) {
    WebSocketClient *client = (WebSocketClient *)timer->privdata;
    client->close();
}

void WebSocketClient::closeAfterTime(int time_ms) {
    if (time_ms <= 0) return;
    event_timer_t timer = add_timer(m_loop_thread->getLoop()->loop(), time_ms, timer_callback, 0);
    timer->privdata = this;
}

}
