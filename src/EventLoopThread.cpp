#include "EventLoopThread.h"
#include <unistd.h>
#include <memory>

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

int EventLoopThread::init() {
    m_eventLoop = new EventLoop();
    if (m_eventLoop->init() != 0) {
        return -1;
    }

    // 使用堆上分配的同步原语
    pthread_mutex_t* mutex = new pthread_mutex_t;
    pthread_cond_t* cond = new pthread_cond_t;
    pthread_mutex_init(mutex, NULL);
    pthread_cond_init(cond, NULL);
    bool* started = new bool(false);

    // 包装参数传递给线程
    struct ThreadArg {
        EventLoop* loop;
        pthread_mutex_t* mutex;
        pthread_cond_t* cond;
        bool* started;
        ~ThreadArg() {  // 析构函数负责清理
            pthread_mutex_destroy(mutex);
            pthread_cond_destroy(cond);
            delete mutex;
            delete cond;
            delete started;
        }
    } thread_arg = {m_eventLoop, mutex, cond, started};

    int ret = pthread_create(&m_threadId, NULL, [](void* arg) -> void* {
        std::unique_ptr<ThreadArg> t_arg(static_cast<ThreadArg*>(arg));
        
        pthread_mutex_lock(t_arg->mutex);
        *t_arg->started = true;
        pthread_cond_signal(t_arg->cond);
        pthread_mutex_unlock(t_arg->mutex);

        t_arg->loop->run();
        return NULL;
    }, new ThreadArg(thread_arg));  // 在堆上创建副本传递给线程

    if (ret != 0) {
        delete mutex;
        delete cond;
        delete started;
        return -1;
    }

    // 等待线程启动
    pthread_mutex_lock(mutex);
    while (!*started) {
        pthread_cond_wait(cond, mutex);
    }
    pthread_mutex_unlock(mutex);

    return 0;
}


}
