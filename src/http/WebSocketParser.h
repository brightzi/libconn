#ifndef WEBSOCKET_PARSER_H
#define WEBSOCKET_PARSER_H

#include <string>
#include <functional>
#include <memory>
#include "ws_util.h"

namespace conn {

#define SEC_WEBSOCKET_KEY "Sec-WebSocket-Key"
#define SEC_WEBSOCKET_ACCEPT "Sec-WebSocket-Accept"
#define SEC_WEBSOCKET_PROTOCOL "Sec-WebSocket-Protocl"

typedef struct ws_frame_header_st {
    uint8_t opcode : 4;
    uint8_t reserved : 3;
    uint8_t fin : 1;

    uint8_t payload_len : 7;
    uint8_t mask : 1;
};

class WebSocketParser {
public:
    WebSocketParser();
    ~WebSocketParser();

    std::function<void(int opcode, const std::string &msg)> onMessage; 

    int feedRecvData(const char *data, size_t len);

    const char *buildFrame(const char *data, size_t len, ws_opcode opcode, int & send_len);

private:
    std::string m_message;
    ws_opcode m_opcode;
    std::string m_buffer;
    char *m_sendBuffer;
};

typedef std::shared_ptr<WebSocketParser> WebSocketParserPtr;

}

#endif