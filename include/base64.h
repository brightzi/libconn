#ifndef BASE_64_H
#define BASE_64_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void base64_encode(const unsigned char *input, size_t len, char *output);

#ifdef __cplusplus
}
#endif

#endif