#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include<pthread.h>
#include<set>
#include<queue>
#include<functional>
#include"../common/mutex.h"
#include"wakeup_fd_event.h"
#include"fd_event.h"
#include"timer.h"

namespace rocket{

class EventLoop{
public:
    EventLoop();
    ~EventLoop();
    void loop();
    void wakeup();
    void stop();
    void addEpollEvent(FdEvent* event);
    void deleteEpollEvent(FdEvent* event);
    bool isInLoopThread();
    void addTask(std::function<void()> cb, bool is_wake_up=false);
    void addTimerEvent(TimerEvent::s_ptr event);
    static EventLoop *GetCurrentEventLoop();
private:
    void dealWakeup();
    void initWakeUpFdEevent();
    void initTimer();
private:
    pid_t m_thread_id{0};
    std::set<int> m_listen_fds;
    int m_epoll_fd{0};
    int m_wakeup_fd{0};
    bool m_stop_flag{false};
    std::queue<std::function<void()>> m_pending_tasks;
    Mutex m_mutex;
    WakeUpFdEvent* m_wakeup_fd_event{NULL};
    Timer *m_timer{NULL};
    bool m_is_looping {false};
};

}

#endif