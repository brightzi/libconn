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
        HttpResponse res;
        req.url = "http://220.181.111.1:80";
        req.method = HTTP_GET;
        req.timeout = 1000000;
        
        int ret = client.send(&req, &res);
        printf("sync resp:%s\n", res.body.c_str());
    }

    return 0;
}