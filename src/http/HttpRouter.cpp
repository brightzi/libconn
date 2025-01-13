#include "HttpRouter.h"

namespace conn {

#define GET_PREFIX "GET_"
#define POST_PREFIX "POST_"
#define PUT_PREFIX "PUT_"
#define DELETE_PREFIX "DELETE_"

HttpRouter::HttpRouter() {

}

HttpRouter::~HttpRouter() {

}

void HttpRouter::get(const char *path, handle_func func) {
    std::string _path = GET_PREFIX + std::string(path);
    addRouter(_path.c_str(), std::move(func));
}

void HttpRouter::post(const char *path, handle_func func) {
    std::string _path = POST_PREFIX + std::string(path);
    addRouter(_path.c_str(), std::move(func));
}

void HttpRouter::put(const char *path, handle_func func) {
    std::string _path = PUT_PREFIX + std::string(path);
    addRouter(_path.c_str(), std::move(func));
}

void HttpRouter::del(const char *path, handle_func func) {
    std::string _path = DELETE_PREFIX + std::string(path);
    addRouter(_path.c_str(), std::move(func));
}

void HttpRouter::addRouter(const char *path, handle_func func) {
    m_path2func[path] = std::move(func);
}

const handle_func *HttpRouter::getHandleFunc(http_method method, const char *path) {
    if (method == HTTP_GET) {
        std::string _path = GET_PREFIX + std::string(path);
        if (m_path2func.find(_path) != m_path2func.end()) {
            return &m_path2func[_path];
        }
    } else if (method == HTTP_POST) {
        std::string _path = POST_PREFIX + std::string(path);
        if (m_path2func.find(_path) != m_path2func.end()) {
            return &m_path2func[_path];
        }
    } else if (method == HTTP_PUT) {
        std::string _path = PUT_PREFIX + std::string(path);
        if (m_path2func.find(_path) != m_path2func.end()) {
            return &m_path2func[_path];
        }
    } else if (method == HTTP_DELETE) {
        std::string _path = DELETE_PREFIX + std::string(path);
        if (m_path2func.find(_path) != m_path2func.end()) {
            return &m_path2func[_path];
        }
    }
    return NULL;
}

}