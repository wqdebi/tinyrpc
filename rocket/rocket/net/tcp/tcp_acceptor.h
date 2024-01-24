#ifndef ROCKER_NET_TCP_TCP_ACCEPT_H
#define ROCKER_NET_TCP_TCP_ACCEPT_H

#include"net_addr.h"

namespace rocket{

class TcpAcceptor{
public:
    TcpAcceptor(NetAddr::s_ptr local_addr);
    ~TcpAcceptor();
    int accept();
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