#ifndef ROCKET_NET_TCP_CONNETCTION
#define ROCKET_NET_TCP_CONNETCTION

#include"rocket/net/tcp/net_addr.h"
#include"rocket/net/tcp/tcp_buffer.h"
#include"rocket/net/io_thread.h"
#include<memory>
#include<queue>
#include"rocket/net/coder/abstract_coder.h"
#include"rocket/net/rpc/rpc_dispatcher.h"

namespace rocket{

enum TcpState{
        NotConnected = 1,
        Connected = 2,
        Halfclosing = 3,
        Closed = 4,
};

enum TcpConnectionType{
    TcpConnectionByServer = 1,//服务端使用
    TcpConnectionByClient = 2,//客户端使用
};

class RpcDispatcher;

class TcpConnection{
public:
    typedef std::shared_ptr<TcpConnection> s_ptr;
public:
    TcpConnection(EventLoop *event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr,TcpConnectionType type = TcpConnectionByServer);
    ~TcpConnection();
    void onRead();
    void excute();
    void onWrite();
    void setState(const TcpState state);
    TcpState getState();
    void clear();
    void shutdown();
    void setConnetctionType(TcpConnectionType type);
    void listenWrite();
    void listenRead();
    void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);
    void pushReadMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done);
    NetAddr::s_ptr getLocalAddr();
    NetAddr::s_ptr getPeerAddr();
private:
    EventLoop *m_event_loop{NULL};
    NetAddr::s_ptr m_locak_addr;
    NetAddr::s_ptr m_peer_addr;
    TcpBuffer::s_ptr m_in_buffer;
    TcpBuffer::s_ptr m_out_buffer;
    FdEvent *m_fd_event{NULL};
    TcpState m_state;
    int m_fd{0};
    TcpConnectionType m_connetction_type{TcpConnectionByServer};
    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;
    std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;
    AbstractCoder* m_coder{NULL};

};

}

#endif