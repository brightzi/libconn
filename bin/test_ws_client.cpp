#include "WebSocketClient.h"
#include <unistd.h>
#include <iostream>
#include <sys/time.h>

using namespace conn;

void onopen() {
    std::cout << "ws onopen" << std::endl;
}

void onMessage(const char *msg, int len) {
    std::cout << "ws onMessage: " << msg << std::endl;
}

void onClose() {
    std::cout << "ws onClose" << std::endl;
    //打印关闭时刻
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    std::cout << "close time: " << t << std::endl;
}

int main(int argc, char *argv[]) {
    WebSocketClient *ws = new WebSocketClient();
    std::map<std::string, std::string> headers;
    // ws->open("http://127.0.0.1:8888", headers);

    // port is ok
    ws->open("ws://124.222.224.186:8800", headers);

    // port is error
    // ws->open("ws://124.222.224.186:8801", headers);

    ws->onopen = onopen;
    ws->onmessage = onMessage;
    ws->onclose = onClose;
    ws->closeAfterTime(3000);

    const char *str = "hello,nihao";
    while(1) {
        // sleep(100000);
        if (str == "exit") {
            ws->close();
            break;
        }
        if (!ws->isConnected()){
            continue;
        } 
        ws->send(str, strlen(str), WS_OPCODE_TEXT);
        usleep(500*1000);
    }

    return 0;
}