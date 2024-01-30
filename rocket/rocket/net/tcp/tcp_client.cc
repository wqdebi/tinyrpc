#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "rocket/common/log.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/fd_event_group.h"
#include "rocket/net/tcp/net_addr.h"

namespace rocket{

TcpClient::TcpClient(NetAddr::s_ptr peer_addr): m_peer_addr(peer_addr){
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamliy(), SOCK_STREAM, 0);
    if(m_fd < 0){
        ERRORLOG("TcpClient::TcpClient() error, failed to  create fd");
        return;
    }
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();
    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr, TcpConnectionByClient);
    m_connection->setConnetctionType(TcpConnectionByClient);
}

TcpClient::~TcpClient(){
    DEBUGLOG("TcpClient::~TcpClient()");
    if(m_fd > 0){
        close(m_fd);
    }
}

void TcpClient::connect(std::function<void()> done){
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if(rt == 0){
        DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
        if(done){
            done();
        }
    }else if(rt == -1){
        if(errno == EINPROGRESS){
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done](){  
                int error = 0;
                socklen_t error_len = sizeof(error);
                bool is_connect_succ = false;
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                if(error == 0){
                    DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                    is_connect_succ = true;
                    m_connection->setState(Connected);
                }else{
                    ERRORLOG("connect error, errno=%d, error=%s",errno, strerror(errno));
                }
                //去掉可写的监听
                m_fd_event->cancle(FdEvent::OUT_EVENT);
                m_event_loop->addEpollEvent(m_fd_event);
                if(is_connect_succ && done){
                    
                    done();
                }
            });
            m_event_loop->addEpollEvent(m_fd_event);
            if(!m_event_loop->isLooping()){
                m_event_loop->loop();
            }
        }else{
            ERRORLOG("connect error,errno=%d, error=%s", errno, strerror(errno));
        }
    }
}

void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done){
    m_connection->pushSendMessage(message, done);
    m_connection->listenWrite();
}

void TcpClient::readMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done){
    m_connection->pushReadMessage(req_id, done);
    m_connection->listenRead();
}

}