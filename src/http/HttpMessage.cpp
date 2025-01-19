#include "HttpMessage.h"
#include <string>

namespace conn {
std::string  HttpResponse::dump(std::string prefix) {
    std::string result;
    result += prefix;
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