#ifndef ROCKET_NET_IO_THREAD_H
#define ROCKET_NET_IO_THREAD_H

#include"eventloop.h"
#include<semaphore.h>

namespace rocket{

class IOThread{
public:
    IOThread();
    ~IOThread();
    static void *Main(void* arg);
    EventLoop *getEventLoop();
    void start();
    void join();
private:
    pthread_t m_thread{0}; //线程号
    pid_t m_thread_id{-1};  //线程句柄
    EventLoop *m_event_loop{NULL}; //当前io线程eventloop对象；
    sem_t m_init_semaphorel;//信号量
    sem_t m_start_semaphorel;
};

}
#endif