#ifndef __C_SSL_H__
#define __C_SSL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void* cssl_ctx_t;
typedef void* cssl_t;

cssl_ctx_t create_ssl_context();

cssl_t ssl_new(cssl_ctx_t ctx, int fd);

int ssl_connect(cssl_t ssl);

int ssl_write(cssl_t ssl, const void *buf, int num);

int ssl_read(cssl_t ssl, void *buf, int len);

int ssl_close(cssl_t ssl);

void ssl_ctx_free(cssl_ctx_t ctx);

void ssl_free(cssl_t ssl);

#ifdef __cplusplus
}
#endif

#endif