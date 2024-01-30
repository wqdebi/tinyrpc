#ifndef ROCKET_NET_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_ABSTRACT_PROTOCOL_H

#include<memory>
#include"rocket/net/tcp/tcp_buffer.h"

namespace rocket{

class AbstractProtocol: public std::enable_shared_from_this<AbstractProtocol>{
public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;
    std::string getReqId(){
        return m_req_id;
    }
    void setReqId(std::string req_id){
        m_req_id = req_id;
    }
    virtual ~AbstractProtocol(){}
protected:
    std::string m_req_id;
};

}

#endif