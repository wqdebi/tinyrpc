#include"tcp_connection.h"
#include "rocket/net/fd_event_group.h"
#include"rocket/common/log.h"
#include<unistd.h>

namespace rocket{

TcpConnection::TcpConnection(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr):
    m_io_thread(io_thread), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd){

    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    io_thread->getEventLoop()->addEpollEvent(m_fd_event);
}

TcpConnection::~TcpConnection(){

}

void TcpConnection::onRead(){
    if(m_state != Connected){
        ERRORLOG("onRead error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    bool is_read_all = false;
    bool is_close = false;
    while(!is_read_all){
        if(m_in_buffer->writeable() == 0){
            m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
        }
        int read_count = m_in_buffer->writeable();
        int write_count = m_in_buffer->writeIndex();
        int rt = read(m_fd, &(m_in_buffer->m_buffer[write_count]), read_count);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);
        if(rt > 0){
            m_in_buffer->moveWriteIndex(rt);
            if(rt == read_count){
                continue;
            }else if(rt < read_count){
                is_read_all = true;
                break;
            }
        }else if(rt == 0){
            is_close = true;
            break;
        }else if(rt == -1 && errno == EAGAIN){
            is_read_all = true;
            break;
        }
    }
    if(is_close){
        //处理关闭连接
        INFOLOG("peer closed, peer addr [%d], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
        clear();
        return;
    }
    if(!is_read_all){
        ERRORLOG("not read all data");
    }
    //简单的echo, 后面补充RPC协议解析
    excute();
}

void TcpConnection::excute(){
    //将RPC请求执行业务逻辑，获取RPC相应，在不RPC相应发送回去
    std::vector<char> tmp;
    int size = m_in_buffer->readable();
    tmp.resize(size);
    m_in_buffer->readFromBuffer(tmp, size);

    std::string msg;
    for(size_t i = 0; i < tmp.size(); ++i){
        msg += tmp[i];  
    }

    INFOLOG("success get request[%s] from client[%s]", msg.c_str(), m_peer_addr->toString().c_str());

    m_out_buffer->writeToBuffer(msg.c_str(), msg.length());
    m_fd_event->listen(FdEvent::OUT_EVENR, std::bind(&TcpConnection::onWrite, this));
    m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
}

void TcpConnection::onWrite(){
    //将当前out_buffer里面数据全部发送给client
    if(m_state != Connected){
        ERRORLOG("onWrite error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }
    bool is_write_all = false;
    while(true){
        if(m_out_buffer->readable() == 0){
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        
        int write_size = m_out_buffer->readable();
        int read_size = m_out_buffer->readIndex();
        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_size]), write_size);
        if(rt >= write_size){
            DEBUGLOG("no data need to send to client [%s]",m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }else if(rt == -1 && errno == EAGAIN){
            ERRORLOG("write data error, errno=EAGAIN and rt == -1"); 
            break;
        }
        
    }
    if(is_write_all){
        m_fd_event->cancle(FdEvent::OUT_EVENR);
        m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
    }
}

void TcpConnection::setState(const TcpState state){
    m_state = Connected;
}

TcpState TcpConnection::getState(){
    return m_state;
}

void TcpConnection::clear(){
    //处理关闭连接的后的清理动作
    if(m_state == Closed){
        return;
    }
    m_fd_event->cancle(FdEvent::IN_EVENT);
    m_fd_event->cancle(FdEvent::OUT_EVENR);
    m_io_thread->getEventLoop()->deleteEpollEvent(m_fd_event);
    m_state = Closed;
}
//服务器主动关闭连接
void TcpConnection::shutdown(){
    if(m_state == Closed || m_state == NotConnected){
        return;
    }
    m_state = Halfclosing;
    //调用shutdown系统函数关闭读和写
    ::shutdown(m_fd, SHUT_RDWR);
}
}  