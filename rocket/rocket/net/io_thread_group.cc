#include"io_thread_group.h"

namespace rocket{

IOThreadGroup::IOThreadGroup(int size){
    m_io_thread_groups.resize(size);
    for(int i = 0; i < size; ++i){
        m_io_thread_groups[i] = new IOThread();
    }
}

IOThreadGroup::~IOThreadGroup(){

}

void IOThreadGroup::start(){
    for(size_t i = 0; i < m_io_thread_groups.size(); ++i){
        m_io_thread_groups[i]->start();
    }
}

IOThread *IOThreadGroup::getIOThread(){
    if(m_index == -1 || m_index == (int)m_io_thread_groups.size()){
        m_index = 0;
    }
    return m_io_thread_groups[m_index++];
}

void IOThreadGroup::join(){
    for(size_t i = 0; i < m_io_thread_groups.size(); ++i){
        m_io_thread_groups[i]->join();
    }
}

}