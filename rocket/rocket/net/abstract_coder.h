#ifndef ROCKET_NET_ABSTRACT_CODER_H
#define ROCKET_NET_ABSTRACT_CODER_H

#include<vector>
#include"rocket/net/tcp/tcp_buffer.h"
#include"rocket/net/abstract_protocol.h"

namespace rocket{

class AbstractCoder{
public:
    virtual void encode(std::vector<AbstractProtocol::s_ptr>& message, TcpBuffer::s_ptr out_buffer) = 0;
    virtual void decode(std::vector<AbstractProtocol::s_ptr>& out_message, TcpBuffer::s_ptr buffer) = 0;
    virtual ~AbstractCoder(){}
};

}

#endif