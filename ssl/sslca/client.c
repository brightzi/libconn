#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 4433
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 1024

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX *create_context() {
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

int main() {
    int sock;
    struct sockaddr_in addr;
    
    SSL_CTX *ctx;
    SSL *ssl;

    init_openssl();  /* 初始化 OpenSSL */
    ctx = create_context();  /* 创建 SSL 上下文 */

    /* 创建 TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr);

    /* 连接服务器 */
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to connect");
        exit(EXIT_FAILURE);
    }

    /* 创建 SSL 连接 */
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    /* 开始 SSL 握手 */
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        /* 向服务器发送加密的消息 */
        SSL_write(ssl, "Hello from client", strlen("Hello from client"));

        /* 接收服务器的响应 */
        char buffer[BUFFER_SIZE] = {0};
        SSL_read(ssl, buffer, BUFFER_SIZE);
        printf("Received message: %s\n", buffer);
    }

    SSL_free(ssl);  /* 释放 SSL 资源 */
    close(sock);  /* 关闭 socket */

    SSL_CTX_free(ctx);  /* 释放 SSL 上下文 */
    cleanup_openssl();  /* 清理 OpenSSL */

    return 0;
}

