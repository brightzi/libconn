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
    if (m_eventLoop->init() != 0) {
        return -1;
    }
    pthread_create(&m_threadId, NULL, runEventLoop, m_eventLoop);
    pthread_detach(m_threadId);
    return 0;
}


}
