#ifndef ROCKET_NET_TCP_CLIENT_H
#define ROCKET_NET_TCP_CLIENT_H

#include <memory>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/abstract_protocol.h"

namespace rocket{

class TcpClient{
public:
    TcpClient(NetAddr::s_ptr peer_addr);
    ~TcpClient();
    //异步进行connect
    //如果connect成功done会执行
    void connect(std::function<void()> done);
    //异步的发送Message
    //如果发送成功会调用done函数。函数的入参就是message对象
    void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);
    //异步的读取Message
    //如果读取成功会调用done函数。函数的入参就是message对象
    void readMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done);
private:
    NetAddr::s_ptr m_peer_addr;
    EventLoop *m_event_loop{NULL};
    int m_fd{-1};
    FdEvent *m_fd_event{NULL};
    TcpConnection::s_ptr m_connection;
};

}

#endif