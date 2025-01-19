#ifndef WS_UTIL_H
#define WS_UTIL_H

#include <stdbool.h>
#include <string.h>
#include <openssl/sha.h> // 用于 SHA-1 计算
#include <openssl/evp.h> // 用于 Base64 编码

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ws_opcode {
    WS_OPCODE_CONTINUE = 0x0,
    WS_OPCODE_TEXT     = 0x1,
    WS_OPCODE_BINARY   = 0x2,
    WS_OPCODE_CLOSE    = 0x8,
    WS_OPCODE_PING     = 0x9,
    WS_OPCODE_PONG     = 0xA
}ws_opcode;

typedef enum ws_session_type {
    WS_CLIENT,
    WS_SERVER,
} ws_session_type;


void ws_encode_key(const char *sec_websocket_key, char ws_accept[32]);

int ws_calc_frame_size(int data_len, bool has_mask);

int build_ws_frame(char *out, const char *data, int data_len, char mask[4], bool has_mask, ws_opcode opcode, bool fin);

#ifdef __cplusplus
}
#endif

#endif