#ifndef ROCKER_NET_TCP_TCP_ACCEPT_H
#define ROCKER_NET_TCP_TCP_ACCEPT_H

#include"net_addr.h"
#include<memory>

namespace rocket{

class TcpAcceptor{
public:
    typedef std::shared_ptr<TcpAcceptor> s_ptr;
public:
    TcpAcceptor(NetAddr::s_ptr local_addr);
    ~TcpAcceptor();
    std::pair<int, NetAddr::s_ptr> accept();
    int getListenFd();
private:
    //服务端监听地址  
    //listenfd
    //adsa
    NetAddr::s_ptr m_local_addr;
    int m_listenfd {-1};
    int m_famliy{-1};
};

}

#endif