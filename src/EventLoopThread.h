#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H
#include "EventLoop.h"
#include <pthread.h>

namespace conn {
class EventLoopThread {
public:
    EventLoopThread();
    virtual ~EventLoopThread();

    int init();

    EventLoop *getLoop() {
        return m_eventLoop;
    }

private:
    // static void *runEventLoop(void *arg);
    EventLoop *m_eventLoop;
    pthread_t m_threadId;

};

};

#endif