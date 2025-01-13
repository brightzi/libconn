#include "HttpMessage.h"
#include <string>

namespace conn {
std::string  HttpResponse::dump() {
    std::string result;
    result += "HTTP/1.1 200 OK\r\n";
    for(const auto &it: headers) {
        result += it.first + ": " + it.second + "\r\n";
    }

    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", body.size());
    result += buf;
    result += "\r\n";
    result += body;
    return std::move(result);
}
}