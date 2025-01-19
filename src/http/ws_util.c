#include "ws_util.h"
#include <stdbool.h>

void ws_encode_key(const char *sec_websocket_key, char ws_accept[32]) {
    const char *guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char concatenated[128];
    unsigned char sha1_hash[SHA_DIGEST_LENGTH]; // SHA_DIGEST_LENGTH 是 20
    char base64_output[64];
    int base64_len;

    // 1. 连接 Sec-WebSocket-Key 和 GUID
    snprintf(concatenated, sizeof(concatenated), "%s%s", sec_websocket_key, guid);

    // 2. 计算 SHA-1 哈希
    SHA1((unsigned char *)concatenated, strlen(concatenated), sha1_hash);

    // 3. Base64 编码
    EVP_EncodeBlock((unsigned char *)base64_output, sha1_hash, SHA_DIGEST_LENGTH);

    // 4. 拷贝结果到 ws_accept
    strncpy(ws_accept, base64_output, 32);
    ws_accept[31] = '\0'; // 确保字符串以 '\0' 结尾
}


int ws_calc_frame_size(int data_len, bool has_mask) {
    int frame_size = 2;  // 基本头部的 2 字节：1 字节用于 FIN + RSV + opcode 和 1 字节用于 MASK + Payload length

    // 根据数据长度计算额外的 Payload 长度字段
    if (data_len < 126) {
        // 数据长度小于 126
        frame_size += 0;  // 不需要额外的长度字段
    } else if (data_len <= 65535) {
        // 数据长度大于等于 126 且小于等于 65535
        frame_size += 2;  // 使用 2 字节表示长度
    } else {
        // 数据长度大于 65535
        frame_size += 8;  // 使用 8 字节表示长度
    }

    // 如果有掩码，则增加 4 字节掩码键
    if (has_mask) {
        frame_size += 4;
    }

    // 最后加上 Payload 数据的大小
    frame_size += data_len;

    return frame_size;
}


int build_ws_frame(char *out, const char *data, int data_len, char mask[4], bool has_mask, ws_opcode opcode, bool fin) {
    int frame_len = 0;
    uint8_t *frame = (uint8_t *)out;

    // 第一个字节：FIN + RSV1, RSV2, RSV3 + Opcode
    frame[frame_len] = (fin ? 0x80 : 0x00) | ((opcode) & 0x0F);
    frame_len++;

    // 第二个字节：Mask + Payload length
    if (data_len < 126) {
        // 数据长度小于 126
        frame[frame_len] = (has_mask ? 0x80 : 0x00) | (data_len & 0x7F);
        frame_len++;
    } else if (data_len <= 65535) {
        // 数据长度大于等于 126，但小于等于 65535
        frame[frame_len] = (has_mask ? 0x80 : 0x00) | 0x7E;
        frame_len++;
        frame[frame_len] = (data_len >> 8) & 0xFF;
        frame[frame_len + 1] = data_len & 0xFF;
        frame_len += 2;
    } else {
        // 数据长度大于 65535
        frame[frame_len] = (has_mask ? 0x80 : 0x00) | 0x7F;
        frame_len++;
        frame[frame_len] = (data_len >> 56) & 0xFF;
        frame[frame_len + 1] = (data_len >> 48) & 0xFF;
        frame[frame_len + 2] = (data_len >> 40) & 0xFF;
        frame[frame_len + 3] = (data_len >> 32) & 0xFF;
        frame[frame_len + 4] = (data_len >> 24) & 0xFF;
        frame[frame_len + 5] = (data_len >> 16) & 0xFF;
        frame[frame_len + 6] = (data_len >> 8) & 0xFF;
        frame[frame_len + 7] = data_len & 0xFF;
        frame_len += 8;
    }

    // 如果有掩码，添加掩码 4 字节
    if (has_mask) {
        memcpy(frame + frame_len, mask, 4);
        frame_len += 4;
    }

    // 添加 Payload 数据并进行掩码处理（如果有掩码）
    if (has_mask) {
        for (int i = 0; i < data_len; i++) {
            frame[frame_len + i] = data[i] ^ mask[i % 4];
        }
    } else {
        memcpy(frame + frame_len, data, data_len);
    }
    frame_len += data_len;

    return frame_len;  // 返回帧的总长度
}


