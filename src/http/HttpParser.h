#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include "http_parser.h"
#include "HttpMessage.h"
#include "httpdef.h"
#include <string>


namespace conn {

class HttpParser {
public:
    HttpParser();
    ~HttpParser();

    int feedRecvData(const char *data, size_t len);

    void handle_header();

    static http_parser_settings cbs;
    std::string url;
    std::string header_field;
    std::string header_value;
    http_headers headers;
    HttpMessage *req;
    HttpMessage *resp;
    http_parser  parser;

    int invokeHttpCb(const char* data = NULL, size_t size = 0);
    int initHttpRequest(HttpMessage *req) {
        if (!req) {
            return -1;
        }
        this->req = req;
        return 0;
    }

    int initHttpResponse(HttpMessage *resp) {
        if (!resp) {
            return -1;
        }
        this->resp = resp;
        return 0;
    }


    http_parser_state state;
    bool isComplete();
};

}

#endif