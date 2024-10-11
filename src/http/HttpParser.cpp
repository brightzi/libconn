#include "HttpParser.h"

namespace conn {

static int on_url(http_parser* parser, const char *at, size_t length);
static int on_status(http_parser* parser, const char *at, size_t length);
static int on_header_field(http_parser* parser, const char *at, size_t length);
static int on_header_value(http_parser* parser, const char *at, size_t length);
static int on_body(http_parser* parser, const char *at, size_t length);
static int on_message_begin(http_parser* parser);
static int on_headers_complete(http_parser* parser);
static int on_message_complete(http_parser* parser);
static int on_chunk_header(http_parser* parser);
static int on_chunk_complete(http_parser* parser);

http_parser_settings HttpParser::cbs = {
    on_message_begin,
    on_url,
    on_status,
    on_header_field,
    on_header_value,
    on_headers_complete,
    on_body,
    on_message_complete,
    on_chunk_header,
    on_chunk_complete
};

int on_url(http_parser* parser, const char *at, size_t length) {
    // printf("on_url: %.*s\n", length, at);
    HttpParser* hp = (HttpParser*)parser->data;
    hp->url.append(at, length);
    hp->state = HP_URL;
    return 0;
}

int on_status(http_parser* parser, const char *at, size_t length) {
    // printf("on_status:%d %.*s\n", (int)parser->status_code, (int)length, at);
    HttpParser* hp = (HttpParser*)parser->data;
    hp->state = HP_STATUS;
    HttpResponse *resp = (HttpResponse *)hp->resp;
    resp->status_code = parser->status_code;
    return 0;
}

int on_header_field(http_parser* parser, const char *at, size_t length) {
    // printf("on_header_field:%.*s\n", (int)length, at);
    HttpParser* hp = (HttpParser*)parser->data;
    hp->handle_header();
    hp->header_field.append(at, length);
    hp->state = HP_HEADER_FIELD;
    return 0;
}

int on_header_value(http_parser* parser, const char *at, size_t length) {
    // printf("on_header_value:%.*s\n", (int)length, at);
    HttpParser* hp = (HttpParser*)parser->data;
    hp->header_value.append(at, length);
    hp->state = HP_HEADER_VALUE;
    return 0;
}

int on_body(http_parser* parser, const char *at, size_t length) {
    // printf("on_body:%d\n", (int)length);
    // printd("on_body:%.*s\n", (int)length, at);
    HttpParser* hp = (HttpParser*)parser->data;
    HttpResponse *resp = (HttpResponse *)hp->resp;
    resp->body.append(at, length);
    hp->state = HP_BODY;
    return 0;
}

int on_message_begin(http_parser* parser) {
    // printf("on_message_begin\n");
    HttpParser* hp = (HttpParser*)parser->data;
    hp->invokeHttpCb();
    hp->state = HP_MESSAGE_BEGIN;
    return 0;
}

int on_headers_complete(http_parser* parser) {
    // printf("on_headers_complete\n");
    HttpParser* hp = (HttpParser*)parser->data;
    hp->handle_header();
    hp->state = HP_MESSAGE_COMPLETE;
    return 0;
}

int on_message_complete(http_parser* parser) {
    // printf("on_message_complete\n");
    HttpParser* hp = (HttpParser*)parser->data;
    hp->invokeHttpCb();
    hp->state = HP_MESSAGE_COMPLETE;
    return 0;
}

int on_chunk_header(http_parser* parser) {
    // printd("on_chunk_header:%llu\n", parser->content_length);
    // Http1Parser* hp = (Http1Parser*)parser->data;
    // int chunk_size = parser->content_length;
    // int reserve_size = MIN(chunk_size + 1, MAX_CONTENT_LENGTH);
    // if (reserve_size > hp->parsed->body.capacity()) {
    //     hp->parsed->body.reserve(reserve_size);
    // }
    // hp->state = HP_CHUNK_HEADER;
    // hp->invokeHttpCb(NULL, chunk_size);
    return 0;
}

int on_chunk_complete(http_parser* parser) {
    // printd("on_chunk_complete\n");
    // Http1Parser* hp = (Http1Parser*)parser->data;
    // hp->state = HP_CHUNK_COMPLETE;
    // hp->invokeHttpCb();
    return 0;
}

HttpParser::HttpParser() {
    http_parser_init(&parser, HTTP_BOTH);
    parser.data = this;
    state = HP_START_REQ_OR_RES;
}

HttpParser::~HttpParser() {
    
}

int HttpParser::feedRecvData(const char *data, size_t len) {
    return http_parser_execute(&parser, &cbs, data, len);
}


int HttpParser::invokeHttpCb(const char* data, size_t size) {
    return 0;
}

bool HttpParser::isComplete() {
    return state == HP_MESSAGE_COMPLETE;
}

void HttpParser::handle_header() {
    if (header_field.size() != 0) {
        headers[header_field] = header_value;
        header_field.clear();
        header_value.clear();
    }
}

int HttpParser::getError() {
    return parser.http_errno;
}
}