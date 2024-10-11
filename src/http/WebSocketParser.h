#ifndef WEBSOCKET_PARSER_H
#define WEBSOCKET_PARSER_H

#include <string>
#include <functional>
#include <memory>

namespace conn {
class WebSocketParser {
public:
    WebSocketParser();
    ~WebSocketParser();

    std::function<void(int opcode, const std::string &msg)> onMessage; 

    int feedRecvData(const char *data, size_t len);

private:
    std::string m_message;
    int m_opcode;
    std::string m_buffer;
};

typedef std::shared_ptr<WebSocketParser> WebSocketParserPtr;

}

#endif