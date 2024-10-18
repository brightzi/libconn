#include "cssl.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

cssl_ctx_t create_ssl_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_client_method();  /* 选择客户端 SSL 协议方法 */
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

cssl_t ssl_new(cssl_ctx_t ctx, int fd) {
    SSL *ssl = SSL_new((SSL_CTX *)ctx);
    if (!ssl) {
        perror("Unable to create SSL");
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    SSL_set_fd(ssl, fd);
    return ssl;
}

int ssl_write(cssl_t ssl, const void *buf, int num) {
    return SSL_write((SSL *)ssl, buf, num);
}

int ssl_read(cssl_t ssl, void *buf, int num) {
    return SSL_read((SSL *)ssl, buf, num);
}

void ssl_free(cssl_t ssl) {
    SSL_free((SSL *)ssl);
}

int ssl_close(cssl_t ssl) {
    SSL_shutdown((SSL *)ssl);
    ssl_free(ssl);
    return 0;
}

void ssl_ctx_free(cssl_ctx_t ctx) {
    SSL_CTX_free((SSL_CTX *)ctx);
}