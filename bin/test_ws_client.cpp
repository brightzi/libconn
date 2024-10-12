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
}

int main(int argc, char *argv[]) {


    WebSocketClient *ws = new WebSocketClient();
    std::map<std::string, std::string> headers;
    // ws->open("http://127.0.0.1:8888", headers);
    ws->open("http://124.222.224.186:8800", headers);
    ws->onopen = onopen;
    ws->onmessage = onMessage;
    ws->onclose = onClose;

    // WebSocketClient *ws2 = new WebSocketClient();
    // ws2->open("http://127.0.0.1:8888", headers);
    // ws2->onopen = onopen;
    // ws2->onmessage = onMessage;
    // ws2->onclose = onClose;

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
        sleep(1);
    }

    return 0;
}