#include <stdio.h>
#include <unistd.h>
#include "HttpClient.h"

using namespace conn;


void onHttpResponse(const HttpResponsePtr&) {

}
int main(int argc, char *argv[]) {
    {
        HttpClient client;

        HttpRequest req;
        req.url = "http://127.0.0.1:8888/echo";
        req.method = conn::HTTP_POST;
        // req.headers["Content-Type"] = "application/json";
        req.timeout = 1000000;
        req.body = "111";

        HttpResponse resp;
        
        int ret = client.async_send(&req, onHttpResponse);
        sleep(1);
        if (ret != 0) {
            return -1;
        }
        while(1) {
            sleep(100000);
        }
        printf("resp: %s\n", resp.body.c_str());
        return 0; 
    }

}