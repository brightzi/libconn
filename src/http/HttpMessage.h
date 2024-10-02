#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H
#include <string>
#include <map>
#include <memory>
#include <functional>

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
};


class HttpResponse: public HttpMessage {
public:
    HttpResponse() {

    }
    virtual ~HttpResponse() {
        
    }

    http_headers headers;
    std::string body;
    HTTP_CODE code;
    void *userdata;

};

typedef std::shared_ptr<HttpRequest>    HttpRequestPtr;
typedef std::shared_ptr<HttpResponse>   HttpResponsePtr;
typedef std::function<void(const HttpResponsePtr&)> HttpResponseCallback;

}

#endif