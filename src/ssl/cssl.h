#ifndef __C_SSL_H__
#define __C_SSL_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

typedef void* cssl_ctx_t;
typedef void* cssl_t;

typedef enum  CSSL_METHOD{
    CSSL_SERVER = 0,
    CSSL_CLIENT = 1
}CSSL_METHOD;

enum {
    CSSL_OK = 0,
    CSSL_ERROR = -1,
    CSSL_WANT_READ = -2,
    CSSL_WANT_WRITE = -3,
    CSSL_WOULD_BLOCK = -4,
};

cssl_ctx_t create_ssl_context(CSSL_METHOD cmethod);

cssl_t ssl_new(cssl_ctx_t ctx, int fd);

int ssl_connect(cssl_t ssl);

int ssl_write(cssl_t ssl, const void *buf, int num);

int ssl_read(cssl_t ssl, void *buf, int len);

int ssl_close(cssl_t ssl);

void ssl_ctx_free(cssl_ctx_t ctx);

void ssl_free(cssl_t ssl);

void ssl_get_peer_cert_chain(cssl_t ssl);

int ssl_accept(cssl_t ssl);

int ssl_get_error(cssl_t ssl, int ret);

void configure_context(cssl_ctx_t *ctx);

#ifdef __cplusplus
}

#endif

#endif