#include"tcp_connection.h"
#include "rocket/net/fd_event_group.h"
#include"rocket/common/log.h"
#include<unistd.h>
#include"rocket/net/string_coder.h"

namespace rocket{

TcpConnection::TcpConnection(EventLoop *event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr,TcpConnectionType type /*= TcpConnectionByServer*/):
    m_event_loop(event_loop), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd), m_connetction_type(type){

    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();
    listenRead();
    if(m_connetction_type == TcpConnectionByServer){
        m_coder = new StringCoder();
    }
    
}

TcpConnection::~TcpConnection(){
    DEBUGLOG("~TcpConnection()");
    if(m_coder){
        delete m_coder;
        m_coder = NULL;
    }
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
    if(m_connetction_type == TcpConnectionByServer){
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
        listenWrite();
    }else{
        std::vector<AbstractProtocol::s_ptr> result;
        m_coder->decode(result, m_in_buffer);
        for(size_t i = 0; i < result.size(); ++i){
            std::string req_id = result[i]->getReqId();
            auto it = m_read_dones.find(req_id);
            if(it != m_read_dones.end()){
                it->second(result[i]);
            }
        }
    }       
}

void TcpConnection::onWrite(){
    //将当前out_buffer里面数据全部发送给client
    if(m_state != Connected){
        ERRORLOG("onWrite error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    if(m_connetction_type == TcpConnectionByClient){
        std::vector<AbstractProtocol::s_ptr> messages;
        for(size_t i = 0; i < m_write_dones.size(); ++i){
            messages.push_back(m_write_dones[i].first);
        }
        m_coder->encode(messages, m_out_buffer);
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
        m_fd_event->cancle(FdEvent::OUT_EVENT);
        m_event_loop->addEpollEvent(m_fd_event);
    }
    if(m_connetction_type == TcpConnectionByClient){
        for(size_t i = 0; i < m_write_dones.size(); ++i){
            m_write_dones[i].second(m_write_dones[i].first);
        }
        m_write_dones.clear();
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
    m_fd_event->cancle(FdEvent::OUT_EVENT);
    m_event_loop->deleteEpollEvent(m_fd_event);
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

void TcpConnection::setConnetctionType(TcpConnectionType type){
    m_connetction_type = type;
}

void TcpConnection::listenWrite(){
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_event_loop->addEpollEvent(m_fd_event);
}
void TcpConnection::listenRead(){
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done){
    m_write_dones.push_back(std::make_pair(message, done));
}

void TcpConnection::pushReadMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done){
    m_read_dones.insert(std::make_pair(req_id, done));
}
}  