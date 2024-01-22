#ifndef ROCKET_NET_IO_THREAD_GROUP_H
#define ROCKET_NET_IO_THREAD_GROUP_H

#include"io_thread.h"
#include<vector>
#include"../common/log.h"

namespace rocket{

class IOThreadGroup{
public:
    IOThreadGroup(int size);
    ~IOThreadGroup();
    void start();
    IOThread *getIOThread();
    void join();
private:
    int m_size{0};
    std::vector<IOThread*> m_io_thread_groups;
    int m_index{0};
};

}

#endif