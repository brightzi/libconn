#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include "HttpMessage.h"
#include "HttpParser.h"

namespace conn {
    
class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    int send(HttpRequest *req, HttpResponse *resp);

    int async_send(HttpRequest *request, HttpResponseCallback callback);

private:
    int parseUrl(const std::string &url, url_t u);

    int exec(HttpRequest *req, HttpResponse *resp);

    bool isTimeout(HttpRequest *req, long start_time, long cur_time);

    std::string makeHttpRequest(HttpRequest *req);

    int connect(const char *ip, int port, int timeout);

    int send_data(int fd, const char *, size_t len);

    int recv_data(int fd, char *buf, size_t len);

private:
    HttpParser *m_parser;

};

}

#endif 