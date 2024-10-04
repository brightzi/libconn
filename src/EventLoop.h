#ifndef EVENT_LOOP_HPP
#define EVENT_LOOP_HPP
#include "event_loop.h"
#include <functional>
#include <queue>
#include <mutex>

namespace conn {

typedef std::function<void(struct Event *)> Functor;

struct Event {
    Functor fn;
    event_st event;
};


class EventLoop {
public:
    EventLoop();
    ~EventLoop();
    
    int init();
    void run();
    void stop();
    void wakeup();
    event_loop_t loop() {
        return m_loop;
    }

    void postEvent(Functor fn);

    std::queue<Event *> m_eventQueue;
    std::mutex m_mutex;

private:
    event_loop_t m_loop;
};

}
#endif