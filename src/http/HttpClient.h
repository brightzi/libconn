#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include "HttpMessage.h"
#include "HttpParser.h"
#include "EventLoopThread.h"
#include "Channel.h"

namespace conn {

typedef struct http_task_st {
    HttpRequest *req;
    HttpResponseCallback cb;
    uint64_t start_time;
    http_task_st() {
    }
    ~http_task_st() {
        printf("task delete\n");
    }
} http_task_st, *http_task_t;
    
class HttpClient {
public:
    HttpClient(EventLoopThread *loop_thread = NULL);
    ~HttpClient();

    int send(HttpRequest *req, HttpResponse *resp);

    int async_send(HttpRequest *request, HttpResponseCallback callback);

private:
    int parseUrl(const std::string &url, url_t u);

    int exec(HttpRequest *req, HttpResponse *resp);

    int connect(const char *ip, int port, int timeout);

    int send_data(int fd, const char *, size_t len);

    int recv_data(int fd, char *buf, size_t len);

    void doTask(http_task_t task);

    void addChannel(io_t io);
    Channel* getChannel(int fd);
    void removeChannel(int fd);
private:
    HttpParser *m_parser;
    EventLoopThread *m_loop_thread;

    std::map<int, Channel *> m_channels;
   
};

}

#endif 