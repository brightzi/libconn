#ifndef WEBSOCKET_PARSER_H
#define WEBSOCKET_PARSER_H

#include <string>
#include <functional>
#include <memory>

namespace conn {

typedef enum ws_opcode {
    WS_OPCODE_CONTINUE = 0x0,
    WS_OPCODE_TEXT     = 0x1,
    WS_OPCODE_BINARY   = 0x2,
    WS_OPCODE_CLOSE    = 0x8,
    WS_OPCODE_PING     = 0x9,
    WS_OPCODE_PONG     = 0xA
} ws_opcode;

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