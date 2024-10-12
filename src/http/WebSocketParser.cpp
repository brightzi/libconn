#include "WebSocketParser.h"
#include <stdlib.h>
#include <string.h>

#define MAX_FRAME_SIZE 4 * 1024 * 1024

namespace conn {

WebSocketParser::WebSocketParser() {
    m_buffer.reserve(MAX_FRAME_SIZE);
    m_sendBuffer = NULL;
}

WebSocketParser::~WebSocketParser() {

}

static inline void ws_ntoh(void *mem, uint8_t len) {
#if __BYTE_ORDER__ != __BIG_ENDIAN
    uint8_t *bytes;
    uint8_t i, mid;

    if (len % 2)
        return;

    mid = len / 2;
    bytes = (uint8_t *) mem;
    for (i = 0; i < mid; i++) {
        uint8_t tmp = bytes[i];
        bytes[i] = bytes[len - i - 1];
        bytes[len - i - 1] = tmp;
    }
#endif
}

static inline void ws_hton(void *mem, uint8_t len) {
#if __BYTE_ORDER__ != __BIG_ENDIAN
    uint8_t *bytes;
    uint8_t i, mid;

    if (len % 2)
        return;

    mid = len / 2;
    bytes = (uint8_t *) mem;
    for (i = 0; i < mid; i++) {
        uint8_t tmp = bytes[i];
        bytes[i] = bytes[len - i - 1];
        bytes[len - i - 1] = tmp;
    }
#endif
}

int WebSocketParser::feedRecvData(const char *data, size_t len) {   
    m_buffer.append(data, len); // Cache the incoming data

    do {
        if (m_buffer.size() < 2) {
            return len;
        }
        struct ws_frame_header_st fh;
        memcpy(&fh, m_buffer.data(), 2);

        size_t offset = 0;
        size_t payload_len = 0;
        if (fh.payload_len < 126) {
            payload_len = fh.payload_len;
            offset += sizeof(struct ws_frame_header_st);
        } else if (fh.payload_len == 126) { 
            uint16_t msg_len;
            if (m_buffer.size() <  sizeof(struct ws_frame_header_st) + sizeof(uint16_t)) {
                return len;
            }
            offset += sizeof(struct ws_frame_header_st);
            memcpy(&msg_len, m_buffer.data() + offset, sizeof(uint16_t));
            ws_ntoh(&msg_len, sizeof(uint16_t));
            offset += sizeof(msg_len);
            payload_len = msg_len;
        } else if (fh.payload_len == 127) {
            uint64_t msg_len;
            if (m_buffer.size() < sizeof(struct ws_frame_header_st) + sizeof(uint64_t)) {
                return len;
            }
            offset += sizeof(struct ws_frame_header_st);
            memcpy(&msg_len, m_buffer.data() + offset, sizeof(uint64_t));
            offset += sizeof(msg_len);
            payload_len = msg_len;
        }

        if (m_buffer.size() < offset + payload_len) {
            return len;
        }

        // Process frame len
        if (fh.mask) {
            uint8_t mask[4];
            if (m_buffer.size() < offset + payload_len + sizeof(mask)/sizeof(mask[0])) {
                return len;
            }
            memcpy(mask, m_buffer.data() + offset, sizeof(mask)/sizeof(mask[0]));
            offset += sizeof(mask) / sizeof(mask[0]);

            for (size_t i = 0; i < payload_len; i++) {
                m_buffer[i+offset] ^= mask[i % 4];
            }

            if (onMessage) {
                std::string_view view(m_buffer.data() + offset, payload_len);
                onMessage(fh.opcode, std::string(view));
                m_buffer.erase(m_buffer.begin(), m_buffer.begin() + offset + payload_len);
            }
        } else { 
            if (onMessage) {
                std::string_view view(m_buffer.data() + offset, payload_len);
                onMessage(fh.opcode, std::string(view));
                m_buffer.erase(m_buffer.begin(), m_buffer.begin() + offset + payload_len);
            }
        }
    }
    while (1);

    return len;
}


const char *WebSocketParser::buildFrame(const char *data, size_t len, ws_opcode opcode, int& send_len) {
    if (len > MAX_FRAME_SIZE) {
        return NULL;
    }
    if (m_sendBuffer == NULL) {
        m_sendBuffer = (char *)malloc(MAX_FRAME_SIZE);
    }

    struct ws_frame_header_st fh = {
        .opcode = (uint8_t)opcode,
        .reserved = 0,
        .fin = 1,
        .payload_len = static_cast<uint8_t>((len > UINT16_MAX) ? 127 : (len > 125) ? 126 : len),
        .mask = 1,
    };

    memset(m_sendBuffer, 0x00, MAX_FRAME_SIZE);
    int offset = 0;
    memcpy(m_sendBuffer, &fh, sizeof(ws_frame_header_st));
    offset += sizeof(ws_frame_header_st);

    if (fh.payload_len == 126) {
        uint16_t msg_len = len;
        ws_hton(&msg_len, sizeof(uint16_t));
        memcpy(m_sendBuffer + offset, &msg_len, sizeof(uint16_t));
        offset += sizeof(uint16_t);
    } else if (fh.payload_len == 127) {
        uint64_t msg_len = len;
        ws_hton(&msg_len, sizeof(uint64_t));
        memcpy(m_sendBuffer + offset, &msg_len, sizeof(uint64_t));
        offset += sizeof(uint64_t);
    }

    char mask[4];
    for(int i = 0; i < 4; i++) {
        mask[i] = rand() % 256;
    }

    memcpy(m_sendBuffer + offset, mask, sizeof(mask));
    offset += 4;

    for (size_t i = 0; i < len; i++) {
        m_sendBuffer[i + offset] = data[i] ^ mask[i % 4];
    }
    send_len = offset + len;
    return m_sendBuffer;
}

};
