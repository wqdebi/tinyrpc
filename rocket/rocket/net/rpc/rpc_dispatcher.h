#ifndef ROCKET_NET_RPC_DISATCHER_H
#define ROCKET_NET_RPC_DISATCHER_H

#include"rocket/net/coder/abstract_protocol.h"
#include<map>
#include<memory>
#include<google/protobuf/service.h>

namespace rocket{

class RpcDispatcher{
public:
    typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;
    void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response);
    void registerService(service_s_ptr service);
private:
    std::map<std::string, service_s_ptr> m_service_map;
};

}

#endif