#include "HttpMessage.h"
#include <string>

namespace conn {

// std::string HttpRequest::dump(bool is_dump_headers, bool is_dump_body) {
//     std::string str;
//     str.reserve(512);
//     // GET / HTTP/1.1\r\n
//     str = asprintf("%s %s HTTP/%d.%d\r\n",
//             url->method->c_str(),
//             proxy ? url.c_str() : path.c_str(),
//             (int)http_major, (int)http_minor);
//     if (is_dump_headers) {
//         DumpHeaders(str);
//     }
//     str += "\r\n";
//     if (is_dump_body) {
//         DumpBody(str);
//     }

// }

// void HttpRequest::dumpHeaders(std::string &str) {
//     }

// }

// void HttpRequest::dumpBody(std::string &str)  {

// }
}