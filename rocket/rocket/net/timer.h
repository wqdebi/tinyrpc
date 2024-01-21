#ifndef ROCKET_NET_TIMER_H
#define ROCKET_NET_TIMER_H

#include<map>
#include"fd_event.h"
#include"timer_event.h"
#include"../common/mutex.h"

namespace rocket{

class Timer: public FdEvent{
public:
    Timer();
    ~Timer();
    void addTimerEvent(TimerEvent::s_ptr event);
    void deleteTimerEvent(TimerEvent::s_ptr event);
    void onTimer(); //发生IO后eventloop会执行这个回调函数
private:
    void resetArriveTime();
private:
    Mutex m_mutex;
    std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;
};

}

#endif