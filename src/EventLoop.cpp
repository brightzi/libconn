#include "EventLoop.h"

namespace conn {

EventLoop::EventLoop() {
    m_loop = nullptr;
}

EventLoop::~EventLoop() {
    if (m_loop) {
        event_loop_destory(m_loop);
        m_loop = nullptr;
    }
}

static void onCustomEvent(event_t ev) {
    EventLoop* loop = (EventLoop*)ev->userdata;
    loop->m_mutex.lock();
    Event * event = loop->m_eventQueue.front();
    loop->m_eventQueue.pop();
    loop->m_mutex.unlock();
    if (event->fn) {
        event->fn(event);
    }
    return ;
}

int EventLoop::init() {
    event_loop_t loop = event_loop_init();
    if (!loop) {
        return -1;
    }
    m_loop = loop;
    return 0;
}

void EventLoop::run() {
    event_loop_run(m_loop);
}
void EventLoop::stop() {
    m_loop->quit = 1;
}
void EventLoop::wakeup() {
    event_loop_wakeup(m_loop);
}

void EventLoop::postEvent(Functor fn) {
    if (m_loop == NULL) {
        return;
    }

    Event *ev = (Event*)malloc(sizeof(struct Event));
    ev->event.userdata = this;
    ev->event.cb = onCustomEvent;
    ev->fn = fn;
    
    m_mutex.lock();
    m_eventQueue.push(ev);
    m_mutex.unlock();
    loop_post_event(m_loop, &ev->event); 
    return ;
}

    
}