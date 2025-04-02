#include <stdio.h>
#include <unistd.h>
#include "HttpClient.h"

using namespace conn;


void onHttpResponse(const HttpResponsePtr& resp) {
    printf("async resp:%s \n", resp->body.c_str());
    return ;
}

int main(int argc, char *argv[]) {
    
    {
        HttpClient client;
        HttpRequest req;
        req.url = "http://127.0.0.1:9988/echo";
        req.method = HTTP_POST;
        req.headers["Content-Type"] = "application/json";
        req.timeout = 1000000;
        req.body = "{\"hello:\":\"world\"}";

        client.async_send(&req, onHttpResponse);
        sleep(5);
    }
    
    return 0;
}