#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 4433
#define BUFFER_SIZE 1024

void init_openssl() {
    SSL_load_error_strings();   /* 加载错误信息 */
    OpenSSL_add_ssl_algorithms();  /* 初始化加密算法库 */
}

void cleanup_openssl() {
    EVP_cleanup();  /* 清理加密算法库 */
}

SSL_CTX *create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();  /* 选择服务器 SSL 协议方法 */
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    /* 加载服务器的证书和私钥 */
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main() {
    int sock;
    struct sockaddr_in addr;
    
    SSL_CTX *ctx;
    SSL *ssl;
    int client;

    init_openssl();  /* 初始化 OpenSSL */
    ctx = create_context();  /* 创建 SSL 上下文 */

    configure_context(ctx);  /* 配置证书和私钥 */

    /* 创建 TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* 绑定地址和端口 */
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    /* 监听端口 */
    if (listen(sock, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    /* 接受客户端连接 */
    client = accept(sock, NULL, NULL);
    if (client < 0) {
        perror("Unable to accept");
        exit(EXIT_FAILURE);
    }

    /* 将客户端的 socket 连接包装为 SSL 连接 */
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client);

    /* 开始 SSL 握手 */
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        /* 读取客户端发送的消息 */
        char buffer[BUFFER_SIZE] = {0};
        SSL_read(ssl, buffer, BUFFER_SIZE);
        printf("Received message: %s\n", buffer);

        /* 发送加密后的响应 */
        SSL_write(ssl, "Hello from server", strlen("Hello from server"));
    }

    SSL_free(ssl);  /* 释放 SSL 资源 */
    close(client);  /* 关闭客户端 socket */
    close(sock);  /* 关闭服务器 socket */

    SSL_CTX_free(ctx);  /* 释放 SSL 上下文 */
    cleanup_openssl();  /* 清理 OpenSSL */

    return 0;
}

