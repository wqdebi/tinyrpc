#ifndef ROCKET_NET_TCP_CONNETCTION
#define ROCKET_NET_TCP_CONNETCTION

#include"rocket/net/tcp/net_addr.h"
#include"rocket/net/tcp/tcp_buffer.h"
#include"rocket/net/io_thread.h"

namespace rocket{

class TcpConnetion{
public:
    enum TcpState{
        NotConnected = 1,
        Conected = 2,
        Halfclosing = 3,
        Closed = 4,
    };
public:
    TcpConnetion(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
    ~TcpConnetion();
    void onRead();
    void excute();
    void onWrite();
private:
    NetAddr::s_ptr m_locak_addr;
    NetAddr::s_ptr m_peer_addr;
    TcpBuffer::s_ptr m_in_buffer;
    TcpBuffer::s_ptr m_out_buffer;
    IOThread* m_io_thread{NULL};
    FdEvent *m_fd_event{NULL};
    TcpState m_state;
    int m_fd{0};
};

}

#endif