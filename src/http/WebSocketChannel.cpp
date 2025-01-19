#include "WebSocketChannel.h"
#include "ws_util.h"
#include <random>

namespace conn {

WebSocketChannel::WebSocketChannel(io_t io, ws_session_type type) : m_buf(NULL), 
m_type(type), m_bufLen(0), Channel(io) {

}

WebSocketChannel::~WebSocketChannel() {
    if (m_buf) {
        free(m_buf);
    }
}

int WebSocketChannel::send(const std::string&msg) {
    std::lock_guard<std::mutex> locker(m_mutex);

    return send(msg.c_str(), msg.size(), WS_OPCODE_TEXT); 
}

int WebSocketChannel::send(const char *data, int len, ws_opcode opcode) {
    std::lock_guard<std::mutex> locker(m_mutex);

    return sendFrame(data, len, opcode, true);
}

int WebSocketChannel::sendFrame(const char *data, int len, ws_opcode opcode, bool fin) {
    bool has_mask = false;
    char mask[4] = {0};
    // 生成随机掩码（4 字节）

    if (m_type == WS_CLIENT) {
        std::random_device rd;
        std::uniform_int_distribution<int> dist(0, 255);  // 随机生成 0 到 255 之间的数 
        for (int i = 0; i < 4; ++i) {
            mask[i] = static_cast<char>(dist(rd));
        }
    }

    int frame_size = ws_calc_frame_size(len, has_mask);
    if (m_bufLen < frame_size) {
        m_bufLen = frame_size * 2;
        m_buf = (char*)realloc(m_buf, m_bufLen);
    }
    build_ws_frame(m_buf, data, len, mask, has_mask, opcode, fin);
    return sendData(m_buf, frame_size);
}

int WebSocketChannel::sendPing() {
    return 0;
}

int WebSocketChannel::sendPong() {
    return 0;
}


}