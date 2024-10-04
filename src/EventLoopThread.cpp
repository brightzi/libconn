#include "EventLoopThread.h"
#include <unistd.h>

namespace conn {

EventLoopThread::EventLoopThread() {
    m_threadId = -1;
}


EventLoopThread::~EventLoopThread() {
    if (m_threadId) {
        pthread_join(m_threadId, NULL);
        m_threadId = -1;
    }
}

static void *runEventLoop(void *arg) {
    EventLoop *loop = (EventLoop *)arg;
    loop->run();
    return NULL;
}

int EventLoopThread::init() {
    m_eventLoop = new EventLoop();
    m_eventLoop->init();
    pthread_create(&m_threadId, NULL, runEventLoop, m_eventLoop);
    sleep(1);
    return 0;
}


}
