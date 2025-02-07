#include "HttpServer.h"
#include <stdio.h>
#include <memory>


using namespace conn;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <http_port> <https_port> \n", argv[0]);
        return 1;
    }

    HttpRouter *router = new HttpRouter();
    router->get("/hello", [](HttpRequest *req, HttpResponse *res) {
        res->headers["Content-Type"] = "text/plain";
        res->headers["Connection"] = "close";
        res->body.append("Hello, World!");
    });

    router->post("/echo", [](HttpRequest *req, HttpResponse *res) {
        res->headers["Content-Type"] = "text/plain";
        res->headers["Connection"] = "close";
        res->body.append(req->body.c_str());
    });

    std::shared_ptr<HttpServer> server = std::make_shared<HttpServer>();
    server->registerHttpRouter(router);
    
    // replace with your own cert and key file
    const char *cert_file = "/home/ubuntu/github/libconn/ssl/sslca/server.crt";
    const char *key_file = "/home/ubuntu/github/libconn/ssl/sslca/server.key";
    server->run("127.0.0.1", argv[1], argv[2], cert_file, key_file);
    return 0;
}