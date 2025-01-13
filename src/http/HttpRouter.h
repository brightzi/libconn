#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H
#include "HttpMessage.h"
#include "http_parser.h"

namespace conn {

typedef std::function<void(HttpRequest *, HttpResponse *)> handle_func;

class HttpRouter {
public:
    HttpRouter();
    virtual ~HttpRouter();

    void get(const char *path, handle_func func);
    void post(const char *path, handle_func func);
    void put(const char *path, handle_func func);
    void del(const char *path, handle_func func);

    const handle_func *getHandleFunc(http_method method, const char *path);

private:
    void addRouter(const char *path, handle_func func);
    std::map<std::string, handle_func> m_path2func;
};

}


#endif