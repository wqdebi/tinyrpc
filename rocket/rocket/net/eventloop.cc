#include"eventloop.h"
#include"../common/log.h"
#include<sys/socket.h>
#include"../common/util.h"
#include<sys/epoll.h>
#include<sys/eventfd.h>
#include"string.h"

#define ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if (it != m_listen_fds.end()) { \
      op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpollEvent(); \
    INFOLOG("epoll_event.events = %d", (int)tmp.events); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
      ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.insert(event->getFd()); \
    DEBUGLOG("add event success, fd[%d]", event->getFd()) \


#define DELETE_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    if (it == m_listen_fds.end()) { \
      return; \
    } \
    int op = EPOLL_CTL_DEL; \
    epoll_event tmp = event->getEpollEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
      ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.erase(event->getFd()); \
    DEBUGLOG("delete event success, fd[%d]", event->getFd()); \



namespace rocket{
static thread_local EventLoop* t_current_event = NULL;
static int g_epoll_max_timeout = 100000;
static int g_epoll_max_events = 10;
EventLoop::EventLoop(){
    if(t_current_event != NULL){
        ERRORLOG("failed to create event loop, this thread has created event loop");
        exit(0);
    }
    m_thread_id = getThreadId();
    m_epoll_fd = epoll_create(10);
    if(m_epoll_fd == -1){
        ERRORLOG("failed to create event loop, epoll_create error, error info [%d]", errno);
        exit(0);
    }

    initWakeUpFdEevent();
    initTimer();

    INFOLOG("suss create event loop in thread %d", m_thread_id);
    t_current_event = this;
}
EventLoop::~EventLoop(){
    close(m_epoll_fd);
    if(m_wakeup_fd_event){
        delete m_wakeup_fd_event;
        m_wakeup_fd_event = NULL;
    }
    if(m_timer){
        delete m_timer;
        m_timer = NULL;
    }
}
void EventLoop::loop(){
    m_is_looping = true;
    //先调用事件，防止有读写操作导致epoll返回的时候才执行
    while (!m_stop_flag)
    {
        ScopeMutex<Mutex> lock(m_mutex);
        std::queue<std::function<void()>> tmp_task;
        m_pending_tasks.swap(tmp_task);
        lock.unlock();
        while(!tmp_task.empty()){
            std::function<void()> cb = tmp_task.front();
            tmp_task.pop();
            if (cb) {
                cb();
            } 
        }
        //如果定时任务需要执行，那么执行
        //如何判断定时任务如何进行
        //arrive_time如何让eventloop监听
        int timeout = g_epoll_max_timeout;
        epoll_event result_event[g_epoll_max_events];
        DEBUGLOG("now begin to epoll_wait");
        int rt = epoll_wait(m_epoll_fd, result_event, g_epoll_max_events, timeout);
        DEBUGLOG("now end epoll_wait, rt=%d", rt);
        if(rt < 0){
            ERRORLOG("epoll_wait error, errno=%d", errno);
        }else{
            for(int i = 0; i < rt; ++i){
                epoll_event trigger_event = result_event[i];
                FdEvent *fd_event = static_cast<FdEvent *>(trigger_event.data.ptr);
                if(fd_event == NULL){
                    continue;
                }
                if(trigger_event.events & EPOLLIN){
                    DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::IN_EVENT), true);
                }
                if(trigger_event.events & EPOLLOUT){
                    addTask(fd_event->handler(FdEvent::OUT_EVENT),true);
                    DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                }
                // if(!(trigger_event.events & EPOLLIN) && !(trigger_event.events & EPOLLOUT)){
                //     int event = (int)trigger_event.events;
                //     DEBUGLOG("unkonow event %d", event);
                // }
                if(trigger_event.events & EPOLLERR){
                    DEBUGLOG("fd %d trigger EPOLLERROR event", fd_event->getFd());
                    if(fd_event->handler(FdEvent::ERROR_EVENT) != nullptr){
                        addTask(fd_event->handler(FdEvent::OUT_EVENT), true);
                    }
                }
            }
        }
    }
}
void EventLoop::wakeup(){
    INFOLOG("WAKE UP");
    m_wakeup_fd_event->wakeup();
}
void EventLoop::stop(){
    m_stop_flag = true;
    wakeup();
}


void EventLoop::addEpollEvent(FdEvent* event){
    if(isInLoopThread()){
        ADD_TO_EPOLL();
    }else{
        auto cb = [this,event](){
            ADD_TO_EPOLL();
        };
        addTask(cb, true);
    }
}
void EventLoop::deleteEpollEvent(FdEvent* event){
    if(isInLoopThread()){
        DELETE_TO_EPOLL();
    }else{
        auto cb = [this, event](){
            DELETE_TO_EPOLL();
        };
        addTask(cb, true);
    }
}

void EventLoop::addTask(std::function<void()> cb, bool is_wake_up){
    ScopeMutex<Mutex> lock(m_mutex);
    m_pending_tasks.push(cb);
    lock.unlock();
    if(is_wake_up){
        wakeup();
    }
}

bool EventLoop::isInLoopThread(){
    return m_thread_id == getThreadId();
}

void EventLoop::initWakeUpFdEevent(){
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
    if(m_wakeup_fd < 0){
        ERRORLOG("failed to create event loop, eventfd create error, error info [%d]", errno);
        exit(0);
    }
    INFOLOG("wakeup fd = %d", m_wakeup_fd);
    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
    m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this](){
        char buf[8];
        while(read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN){
        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
    });
    addEpollEvent(m_wakeup_fd_event);
}
void EventLoop::addTimerEvent(TimerEvent::s_ptr event){
    m_timer->addTimerEvent(event);
}
void EventLoop::initTimer(){
    m_timer = new Timer();
    addEpollEvent(m_timer);
}

EventLoop *EventLoop::GetCurrentEventLoop(){
    if(t_current_event) {
        return t_current_event;
    }
    t_current_event = new EventLoop();
    return t_current_event;
}

bool EventLoop::isLooping(){
    return m_is_looping;
}

}