#include "HttpClient.h"
#include "timer.h"
#include "conn_socket.h"
#include "io.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <functional> 

namespace conn {

HttpClient::HttpClient(EventLoopThread *loop_thread):
    m_parser(NULL),
    m_loop_thread(loop_thread) {
    m_parser = new HttpParser();
}

HttpClient::~HttpClient() {
    if (m_parser) {
        delete m_parser;
        m_parser = NULL;
    }
}

static void fill_http_header(HttpRequest *req, std::string &data) {
    if (req->method == HTTP_GET) {
        data += "GET /" + std::string(req->u->path) + " HTTP/1.1\r\n";
    } else if (req->method == HTTP_POST) {
        data += "POST /" + std::string(req->u->path) + " HTTP/1.1\r\n";
    }

    data += "Host: " + std::string(req->u->host) + "\r\n";
    data += "User-Agent: libconn\r\n"; 
    data += "Accept: */*\r\n";
    data += "Content-Type: application/x-www-form-urlencoded\r\n";
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", req->body.size());
    data += buf;
    data += "\r\n";
    return ;
}

static void fill_http_body(HttpRequest *req, std::string &data) {
    if (req->body.empty()) {
        return ;
    }
    data += req->body;
    return ;
}

static std::string makeHttpRequest(HttpRequest *req) {
    std::string data;
    fill_http_header(req, data);
    if (req->method == HTTP_POST) {
        fill_http_body(req, data);
    }
    return data;
}


static bool isTimeout(HttpRequest *req, long start_time, long cur_time) {
    assert(req != NULL);
    if (cur_time - start_time > req->timeout) {
        return true;
    }
    return false;
}

int HttpClient::send(HttpRequest *req, HttpResponse *resp) {
    if (req == NULL || resp == NULL) {
        return -1;
    }

    if (req->url.empty()) {
        return -1;
    }

    if (parseUrl(req->url, req->u) != 0) {
        return -1;
    }

    int ret = exec(req, resp); 
    if (ret != 0) {
        return ret;
    }
    return 0;
}

int HttpClient::async_send(HttpRequest *req, HttpResponseCallback callback) {
    if (req == NULL) {
        return -1;
    }

    if (req->url.empty()) {
        return -1;
    }

    if (parseUrl(req->url, req->u) != 0) {
        return -1;
    }

    if (m_loop_thread == NULL) {
        m_loop_thread = new EventLoopThread();
        m_loop_thread->init();
        usleep(10000);
    }

    http_task_t task = new http_task_st();
    if (task == NULL) {
        return -1;
    }
    task->req = req;
    task->cb = std::move(callback);
    task->start_time = get_curtime_ms();
    m_loop_thread->getLoop()->postEvent(std::bind(&HttpClient::doTask, this, task));
    return 0;
}

int HttpClient::parseUrl(const std::string &url, url_t u) {
    char temp[1024] = {0};
    strcpy(temp, url.c_str());
    char *scheme_ptr = strstr(temp, "://");
    if (scheme_ptr == NULL) {
        return -1;
    }
    *scheme_ptr = '\0';
    strcpy(u->scheme, temp);

    char *host_ptr = scheme_ptr + 3;
    char *port_ptr = strchr(host_ptr, ':');
    if (port_ptr != NULL) {
        *port_ptr = '\0';
        strcpy(u->host, host_ptr);
        host_ptr = port_ptr + 1;
    } else {
        return 0;
    }

    // 解析端口和路径
    char *path_ptr = strchr(host_ptr, '/');
    if (path_ptr != NULL) {
        *path_ptr = '\0';
        u->port = atoi(host_ptr);
        strcpy(u->path, path_ptr+1);
    } else {
        u->port = atoi(host_ptr);
        strcpy(u->path, "/");
    }

    return 0;
}

int HttpClient::connect(const char *ip, int port, int timeout) {

}

int HttpClient::exec(HttpRequest *req, HttpResponse *resp) {
    long start_time = get_curtime_ms();

    std::string data = makeHttpRequest(req);
    if (data.empty()) {
        return -1;
    }

    int connfd = connectWithTimeout(req->u->host, req->u->port, req->timeout);
    if (connfd < 0) {
        return -1;
    }

    if (isTimeout(req, start_time, get_curtime_ms())) {
        return -1; 
    }

    size_t len = data.size();
    int have_send_len = 0;
    const char *send_buf = data.c_str();
    while(have_send_len != len) {
        int nsend = send_data(connfd, send_buf + have_send_len, len - have_send_len);
        if (isTimeout(req, start_time, get_curtime_ms())) {
            return -1;
        }
        have_send_len += nsend;
    }

    // m_parser->initHttpRequest(req);
    m_parser->initHttpResponse(resp);

    char buf[1024] = {0};
    do {
        int nrecv = recv_data(connfd, buf, sizeof(buf));
        // nread +=1;
        if (nrecv <= 0) {
            if (isTimeout(req, start_time, get_curtime_ms())) {
                break;
            }
            if (errno == EINTR) {
                usleep(10 * 1000);
                continue;
            } else {
                closesocket(connfd);
                return -1;
            }
        }
        int nparse = m_parser->feedRecvData(buf, nrecv);
        if (nparse != nrecv) {
            closesocket(connfd);
            return -1;
        }
    }while(!m_parser->isComplete());
    closesocket(connfd);
    return 0;
}

int HttpClient::send_data(int fd, const char *buf, size_t len) {
    return ::send(fd, buf, len, 0);
}

int HttpClient::recv_data(int fd, char *buf, size_t len) {
    return ::recv(fd, buf, len, 0);
}


//async http client
static void sendHttpRequest(Channel *channel) {
    http_task_t task = (http_task_t)channel->m_ctx;
    std::string data = makeHttpRequest(task->req);
    if (data.empty()) {
        return ;
    }

    if (isTimeout(task->req, task->start_time, get_curtime_ms())) {
        return ;
    }
    channel->sendData(data.c_str(), data.size());
}

void HttpClient::doTask(http_task_t task) {
    printf("\n\ndotask:%s\n", task->req->u->path);
    char port[16] = {0};
    snprintf(port, sizeof(port), "%d", task->req->u->port);
    int block = 0;
    int client_fd = create_socket(block);
    io_t io = get_io(m_loop_thread->getLoop()->loop(), client_fd);
    if (io == NULL) {
        return ;
    }
    io->ip = strdup(task->req->u->host);
    io->port = strdup(port);
    addChannel(io);
    Channel *channel = getChannel(client_fd);
    channel->m_ctx = task;
    io->ctx = channel;

    channel->onconnect = [&channel]() {
        printf("httpclient onconnect\n");
        sendHttpRequest(channel);
    };

    channel->onread = [&io](Buffer *buf) {
        const char *data = buf->view_data();
        int size = buf->readable_size();
        printf("readdata : %s", data);
    };

    channel->onwrite = [&io](Buffer *buf) {

    };

    channel->onclose = [this, &io]() {
        removeChannel(io->fd);
    };

    channel->startConnect();

    return ;
}

void HttpClient::addChannel(io_t io) {
    Channel *channel = new Channel(io);    
    channel->init();
    m_channels[io->fd] = channel;
}

Channel* HttpClient::getChannel(int fd) {
    return m_channels[fd];
}

void HttpClient::removeChannel(int fd) {
    Channel *channel = m_channels[fd];
    m_channels.erase(fd);
    delete channel;
}
    
}
    