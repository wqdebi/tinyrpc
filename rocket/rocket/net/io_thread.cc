#include"io_thread.h"
#include"../common/log.h"
#include<pthread.h>
#include"../common/util.h"
#include<assert.h>

namespace rocket{

IOThread::IOThread(){
    int rt = sem_init(&m_init_semaphorel, 0, 0);
    assert(rt == 0);

    rt = sem_init(&m_start_semaphorel, 0, 0);
    assert(rt == 0);

    pthread_create(&m_thread, NULL, &IOThread::Main, this);
    //等到新线程执行完Main函数的前置

    sem_wait(&m_init_semaphorel);
    DEBUGLOG("IOThread [%d] create success", m_thread_id);
}

IOThread::~IOThread(){
    m_event_loop->stop();
    sem_destroy(&m_init_semaphorel);
    sem_destroy(&m_start_semaphorel);
    pthread_join(m_thread, NULL);
    if(m_event_loop){
        delete m_event_loop;
        m_event_loop = NULL;
    }
}

void *IOThread::Main(void* arg){
    //前置部分
    IOThread *thread = static_cast<IOThread *>(arg);
    thread -> m_event_loop = new EventLoop();
    thread -> m_thread_id = getThreadId();

    //循环部分
    //唤醒等待
    sem_post(&thread->m_init_semaphorel);
    DEBUGLOG("IOThread %d create, wait start semaphre", thread->m_thread_id);
    sem_wait(&thread->m_start_semaphorel);
    DEBUGLOG("IOThread %d start loop", thread->m_thread_id);
    thread -> m_event_loop -> loop();
    DEBUGLOG("IOThread %d end loop", thread->m_thread_id);
    return NULL;
}

EventLoop *IOThread::getEventLoop(){
    return m_event_loop;
}

void IOThread::start(){
    DEBUGLOG("Now invoke IOThread %d", m_thread_id);
    sem_post(&m_start_semaphorel);
}

void IOThread::join(){
    pthread_join(m_thread, NULL);
}

}