# 指定源文件
set(SOURCES 
    event_loop.c
    event.c
    io.c
    base64.c
    conn_socket.c
    EventLoop.cpp
    EventLoopThread.cpp
    Channel.cpp
    Buffer.cpp
    ./dispatcher/select.c
    ./http/HttpClient.cpp
    # ./http/HttpMessage.cpp
    ./http/HttpParser.cpp
    ./http/http_parser.c
    ./http/WebSocketClient.cpp
    ./http/WebSocketParser.cpp
    ./ssl/open_ssl.c
)

# 创建静态库
add_library(${CONN_LIB} STATIC ${SOURCES})