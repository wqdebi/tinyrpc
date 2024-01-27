#include"tcp_connection.h"
#include "rocket/net/fd_event_group.h"
#include"rocket/common/log.h"
#include<unistd.h>

namespace rocket{

TcpConnetion::TcpConnetion(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr):
    m_io_thread(io_thread), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd){
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnetion::onRead, this));
}

TcpConnetion::~TcpConnetion(){

}

void TcpConnetion::onRead(){
    if(m_state != Conected){
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
        }else{
            is_close = true;
        }
    }
    if(is_close){
        //处理关闭连接
        INFOLOG("peer closed, peer addr [%d], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
    }
    if(!is_read_all){
        ERRORLOG("not read all data");
    }
    //简单的echo, 后面补充RPC协议解析
    excute();
}

void TcpConnetion::excute(){
    //将RPC请求执行业务逻辑，获取RPC相应，在不RPC相应发送回去
    std::vector<char> tmp;
    int size = m_in_buffer->readable();
    tmp.resize(size);
    m_in_buffer->readFromBuffer(tmp, size);

    std::string msg;
    for(int i = 0; i < tmp.size(); ++i){
        msg += tmp[i];
    }

    INFOLOG("success get request[%s] from client[%s]", msg.c_str(), m_peer_addr->toString().c_str());

    m_out_buffer->writeToBuffer(msg.c_str(), msg.length());
    m_fd_event->listen(FdEvent::OUT_EVENR, std::bind(&TcpConnetion::onWrite, this));
}

void TcpConnetion::onWrite(){
    //将当前out_buffer里面数据全部发送给client
    if(m_state != Conected){
        ERRORLOG("onWrite error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }
    while(true){
        if(m_out_buffer->readable() == 0){
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            break;
        }
        int write_size = m_out_buffer->readable();
        int read_size = m_out_buffer->readIndex();
        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_size]), write_size);
        if(rt >= write_size){
            DEBUGLOG("no data need to send to client [%s]",m_peer_addr->toString().c_str());
            break;
        }else if(rt == -1 && errno == EAGAIN){
            ERRORLOG("write data error, errno=EAGAIN and rt == -1");
            break;
        }
    }
}


}