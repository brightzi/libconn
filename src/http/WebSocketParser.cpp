#include "WebSocketParser.h"
#include <string.h>

namespace conn {

WebSocketParser::WebSocketParser() {

}

WebSocketParser::~WebSocketParser() {

}

int WebSocketParser::feedRecvData(const char *data, size_t len) {   
    m_buffer.append(data, len); // Cache the incoming data

    while (m_buffer.size() >= 2) { // At least two bytes needed for a frame header
        // Parse the first byte for FIN and opcode
        uint8_t firstByte = m_buffer[0];
        bool fin = (firstByte & 0x80) != 0; // FIN bit
        m_opcode = firstByte & 0x0F; // Opcode

        // Parse the second byte for masking and payload length
        uint8_t secondByte = m_buffer[1];
        bool masked = (secondByte & 0x80) != 0; // Mask bit
        size_t payloadLen = secondByte & 0x7F;

        size_t offset = 2;
        if (payloadLen == 126) {
            if (m_buffer.size() < 4) return 0; // Wait for more data
            payloadLen = (m_buffer[2] << 8) | m_buffer[3];
            offset += 2;
        } else if (payloadLen == 127) {
            if (m_buffer.size() < 10) return 0; // Wait for more data
            // 8-byte length (big-endian) - ignored for now for simplicity
            payloadLen = (static_cast<size_t>(m_buffer[2]) << 56) |
                         (static_cast<size_t>(m_buffer[3]) << 48) |
                         (static_cast<size_t>(m_buffer[4]) << 40) |
                         (static_cast<size_t>(m_buffer[5]) << 32) |
                         (static_cast<size_t>(m_buffer[6]) << 24) |
                         (static_cast<size_t>(m_buffer[7]) << 16) |
                         (static_cast<size_t>(m_buffer[8]) << 8) |
                         (static_cast<size_t>(m_buffer[9]));
            offset += 8;
        }

        // Check if we have enough data for the payload
        if (m_buffer.size() < offset + payloadLen) {
            return 0; // Wait for more data
        }

        // Unmask the payload if needed
        if (masked) {
            uint8_t maskingKey[4];
            memcpy(maskingKey, m_buffer.data() + offset, 4);
            offset += 4;

            m_message.resize(payloadLen);
            for (size_t i = 0; i < payloadLen; ++i) {
                m_message[i] = m_buffer[offset + i] ^ maskingKey[i % 4];
            }
        } else {
            m_message.assign(m_buffer.data() + offset, payloadLen);
        }

        // Call the message handler
        if (onMessage) {
            onMessage(m_opcode, m_message);
        }

        // Remove processed bytes from the buffer
        size_t bytesProcessed = offset + payloadLen;
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + bytesProcessed);
    }

    return static_cast<int>(len);
}

};
