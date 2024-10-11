#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <string.h>

namespace conn {
typedef enum HTTP_METHOD {
    HTTP_GET,
    HTTP_POST,
    HTTP_UNKNOWN
} HTTP_METHOD;

typedef enum HTTP_CODE {
    HTTP_OK = 200,
    HTTP_BAD_REQUEST = 400,
    HTTP_NOT_FOUND = 404,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_UNKNOWN_CODE = 999
} HTTP_CODE;

struct url_st {
    char scheme[8];
    char host[32];
    int port;
    char path[256];
};

typedef std::map<std::string, std::string> http_headers;
typedef struct url_st url_st, *url_t;

class HttpMessage {
public:
    HttpMessage() {

    }

    virtual ~HttpMessage() {
        
    }
};

class HttpRequest : public HttpMessage {
public:
    HttpRequest() {
        u =(url_t) malloc(sizeof(url_st));
    }

    virtual ~HttpRequest() {

    }

    int parseUrl() {
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

    std::string url;
    url_t u;
    HTTP_METHOD method;
    http_headers headers;
    std::string body;
    int retry; 
    int timeout;

    void setRetry(int retry) {
        this->retry = retry;
    }

    void setTimeout(int timeout) {
        this->timeout = timeout;
    }

    std::string dump(bool is_dump_headers, bool is_dump_body);

    void dumpHeaders(std::string &str);

    void dumpBody(std::string &str);

};


class HttpResponse: public HttpMessage {
public:
    HttpResponse() {

    }
    virtual ~HttpResponse() {
        
    }

    const std::string & getHeader(const std::string& key) {
        if (headers.find(key) == headers.end()) {
            return std::string("");
        }
        return headers[key];
    }

    http_headers headers;
    std::string body;
    HTTP_CODE code;
    int status_code;
    void *userdata;
};

typedef std::shared_ptr<HttpRequest>    HttpRequestPtr;
typedef std::shared_ptr<HttpResponse>   HttpResponsePtr;
typedef std::function<void(const HttpResponsePtr&)> HttpResponseCallback;

}

#endif