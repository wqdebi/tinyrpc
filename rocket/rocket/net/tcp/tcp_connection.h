#ifndef ROCKET_NET_TCP_CONNETCTION
#define ROCKET_NET_TCP_CONNETCTION

#include"rocket/net/tcp/net_addr.h"
#include"rocket/net/tcp/tcp_buffer.h"
#include"rocket/net/io_thread.h"
#include<memory>

namespace rocket{

enum TcpState{
        NotConnected = 1,
        Connected = 2,
        Halfclosing = 3,
        Closed = 4,
};

class TcpConnection{
public:
    typedef std::shared_ptr<TcpConnection> s_ptr;
public:
    TcpConnection(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
    ~TcpConnection();
    void onRead();
    void excute();
    void onWrite();
    void setState(const TcpState state);
    TcpState getState();
    void clear();
    void shutdown();
private:
    IOThread* m_io_thread{NULL};
    NetAddr::s_ptr m_locak_addr;
    NetAddr::s_ptr m_peer_addr;
    TcpBuffer::s_ptr m_in_buffer;
    TcpBuffer::s_ptr m_out_buffer;
    FdEvent *m_fd_event{NULL};
    TcpState m_state;
    int m_fd{0};
};

}

#endif