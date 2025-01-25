#include "HttpHandle.h"
#include "event_loop.h"
#include "ws_util.h"

namespace conn {

HttpHandle::HttpHandle(io_t io) : m_router(NULL),
m_parser(NULL), m_request(NULL), m_response(NULL),
m_wsParser(NULL), m_wsChannel(NULL), m_io(io){
    m_serviceType = SERVICE_HTTP;
}

HttpHandle::~HttpHandle() {
    close();

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
    
    if (m_wsParser) {
        delete m_wsParser;
        m_wsParser = NULL;
    }
    
    if (m_wsChannel) {
        delete m_wsChannel;
    }
}

int HttpHandle::setRouter(HttpRouter *router, WebSocketService * ws_service) {
    m_router = router;
    m_wsService = ws_service;

    if (m_parser == NULL) {
        m_parser = new HttpParser(HTTP_PARSER_REQUEST);
    }
    m_request = new HttpRequest();
    m_response = new HttpResponse();
    if (m_parser->initHttpRequest(m_request) != 0) {
        return -1;
    }

    if (m_parser->initHttpResponse(m_response) != 0) {
        return -1;
    }
    m_wsParser = new WebSocketParser();
    return 0;
}

int HttpHandle::feedRecvData(const char *data, int len) {   
    if (m_serviceType == SERVICE_HTTP) {
        int nparse = m_parser->feedRecvData(data, len);
        if (nparse != len) {
            return -1;
        }
        if (m_parser->isComplete()) {
            if (isUpgradeWSProtocol()) {
                switchWSProtocol();
            } else {
                sendHttpResponse();
            }
        }
    } else if (m_serviceType == SERVICE_WEBSOCKET) {
        m_wsParser->feedRecvData(data, len);
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
    if (m_serviceType == SERVICE_HTTP) {
        if (invokeHttpFunc() != 0) {
            return ;
        }

        const std::string &data = m_response->dump(HTTP_HEADER);
        io_send_data(m_io, data.c_str(), data.size());
    } else if (m_serviceType == SERVICE_WEBSOCKET) {
        const std::string &data = m_response->dump(WS_HEADER);
        io_send_data(m_io, data.c_str(), data.size());
    }
    return ;
}

bool HttpHandle::isUpgradeWSProtocol() {
    auto &headers = m_request->headers;
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        if (strcasecmp(it->first.c_str(), "Upgrade") == 0 &&
         strcasecmp(it->second.c_str(), "websocket") == 0) {
            return true;
        }
    }
    return false;
}

bool HttpHandle::switchWSProtocol() {
    upgradeWSProtocol();
    m_wsChannel = new WebSocketChannel(m_io, WS_SERVER);
    if (m_wsService) {
        m_wsService->onopen(m_wsChannel, m_request);
    }
    m_wsParser->onMessage = [this](int op_code, const std::string &msg) {
        ws_opcode opcode = (enum ws_opcode)op_code;
        switch(opcode) {
            case WS_OPCODE_TEXT:
            case WS_OPCODE_BINARY:
                if (m_wsService) {
                    m_wsService->onmessage(m_wsChannel, msg.c_str(), msg.size(), opcode);
                }
                break;
            case WS_OPCODE_CLOSE:
                m_wsChannel->send(msg.c_str(), msg.size(), WS_OPCODE_CLOSE);
                m_wsChannel->close();
                break;
            case WS_OPCODE_PING:
                m_wsChannel->send("",0, WS_OPCODE_PONG);
                break;
            case WS_OPCODE_PONG:
                break;
            default:
                break;
        }
    };
    return true;
}

bool HttpHandle::upgradeWSProtocol() {
    m_response->status_code = HTTP_STATUS_SWITCHING_PROTOCOLS;
    m_response->headers["Connection"] = "Upgrade";
    m_response->headers["Upgrade"] = "websocket";
    

    auto iter_key = m_request->headers.find(SEC_WEBSOCKET_KEY);
    if (iter_key != m_request->headers.end()) {
        char ws_accept[32] = {0};
        ws_encode_key(iter_key->second.c_str(), ws_accept);
        m_response->headers[SEC_WEBSOCKET_ACCEPT] = ws_accept;
    }

    auto iter_protocol = m_request->headers.find(SEC_WEBSOCKET_PROTOCOL);
    if (iter_protocol != m_request->headers.end()) {
        m_response->headers[SEC_WEBSOCKET_PROTOCOL] = iter_protocol->second;
    }
    printf("switch to websocket\n");
    m_serviceType = SERVICE_WEBSOCKET;
    sendHttpResponse();
    return true;
}


void HttpHandle::close() {
    if (m_serviceType == SERVICE_WEBSOCKET) {
        if (m_wsService) {
            m_wsService->onclose(m_wsChannel);
        }
    }
    return ;
}



};