#include"tcp_buffer.h"
#include"string.h"
#include"rocket/common/log.h"
#include<memory>

namespace rocket{

TcpBuffer::TcpBuffer(int size): m_size(size){
    m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer(){

}

int TcpBuffer::readable(){
    return m_write_index - m_read_index;
}
int TcpBuffer::writeable(){
    return m_size - m_write_index;
}

int TcpBuffer::readIndex(){
    return m_read_index;
}

int TcpBuffer::writeIndex(){
    return m_write_index;
}

void TcpBuffer::writeToBuffer(const char* buf, int size){
    if(size > writeable()){
        int new_size = (int)(1.5 * (m_write_index + size));
        resizeBuffer(new_size);
    }
    mempcpy(&m_buffer[m_write_index], buf, size);
    m_write_index += size;
}

void TcpBuffer::readFromBuffer(std::vector<char> &re, int size){
    if(readable() == 0)
        return;
    int read_size = readable() > size ? size : readable();
    std::vector<char> tmp(read_size);
    memcpy(&tmp[0], &m_buffer[m_read_index], read_size);
    re.swap(tmp);
    m_read_index += read_size;
    adjustBuffer();
}

void TcpBuffer::resizeBuffer(int new_size){
    std::vector<char> tmp;
    int count = std::min(new_size, readable());
    memcpy(&tmp[0], &m_buffer[m_read_index], count);
    m_buffer.swap(tmp);
    m_read_index = 0;
    m_write_index = m_read_index + count;
    m_size = new_size;
}

void TcpBuffer::adjustBuffer(){
    if(m_read_index < int(m_buffer.size() / 3)){
        return;
    }
    std::vector<char> buffer(m_buffer.size());
    int count = readable();
    memcpy(&buffer[0], &m_buffer[m_read_index], count);
    m_buffer.swap(buffer);
    m_read_index = 0;
    m_write_index = m_read_index + count;
    buffer.clear();
}

void TcpBuffer::moveReadIndex(int size){
    size_t j = m_read_index + size;
    if(j >= m_buffer.size()){
        ERRORLOG("moveReadIndex error, invalid size %d, old real index %d, buffer size %d", size, m_read_index, m_buffer.size());
        return;
    }
    m_read_index = j;
    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size){
    size_t j = m_write_index + size;
    if(j >= m_buffer.size()){
        ERRORLOG("moveWriteIndex error, invalid size %d, old real index %d, buffer size %d", size, m_read_index, m_buffer.size());
        return;
    }
    m_write_index = j;
    adjustBuffer();
}

}