#include "cssl.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

cssl_ctx_t create_ssl_context(CSSL_METHOD cmethod) {
    static int init = 0; 
    if (!init) {
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
    }

    const SSL_METHOD *method;
    SSL_CTX *ctx;

    if (cmethod == CSSL_SERVER) {
        method = SSLv23_server_method();  /* 选择服务器端 SSL 协议方法 */
    } else {
        method = SSLv23_client_method();  /* 选择客户端 SSL 协议方法 */
    }
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

int ssl_connect(cssl_t ssl) {
    return SSL_connect((SSL *)ssl);
}

int ssl_write(cssl_t ssl, const void *buf, int num) {
    int nwrite;
    if ((nwrite = SSL_write((SSL *)ssl, buf, num)) < 0) {
        unsigned long err_code = ERR_get_error();
        char err_msg[120];
        ERR_error_string_n(err_code, err_msg, sizeof(err_msg));
        fprintf(stderr, "SSL_write error: %s\n", err_msg);
    }
    return nwrite;
}

int ssl_read(cssl_t ssl, void *buf, int num) {
    int nread; 
    if ((nread = SSL_read((SSL *)ssl, buf, num))< 0) {
        unsigned long err_code = ERR_get_error();
        char err_msg[120];
        ERR_error_string_n(err_code, err_msg, sizeof(err_msg));
        fprintf(stderr, "SSL_read error: %s\n", err_msg);
        return -1;
    }
    return nread;
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


void ssl_get_peer_cert_chain(cssl_t ssl) {
    STACK_OF(X509) *cert_chain = SSL_get_peer_cert_chain(ssl);
    for (int i = 0; i < sk_X509_num(cert_chain); i++) {
        X509 *cert = sk_X509_value(cert_chain, i);
        printf("Certificate #%d:\n", i + 1);

        // 将证书打印为可读的信息
        X509_print_fp(stdout, cert);
        
        // 将证书转换为 PEM 格式并打印
        printf("\nPEM format:\n");
        PEM_write_X509(stdout, cert);  // 将证书以 PEM 格式输出到标准输出
        printf("\n");
    }
}

int ssl_get_error(cssl_t ssl, int ret) {
    int err = SSL_get_error((SSL *)ssl, ret);
    if (err == SSL_ERROR_WANT_READ) {
        return CSSL_WANT_READ;
    } else if (err == SSL_ERROR_WANT_WRITE) {
        return CSSL_WANT_WRITE;
    }
    return CSSL_ERROR;
}

int ssl_accept(cssl_t ssl) {
    int ret = SSL_accept((SSL*)ssl);
    return (ret == 1) ? 0 : ssl_get_error(ssl, ret);
}

void configure_context(cssl_ctx_t *ctx, const char *cert_file, const char *key_file) {
     /* 加载服务器的证书和私钥 */
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}