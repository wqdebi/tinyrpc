#ifndef ROCKET_NET_RPC_CLOSURE_H
#define ROCKET_NET_RPC_CLOSURE_H

#include<google/protobuf/stubs/callback.h>
#include<functional>

namespace rocker{

class RpcClosure: google::protobuf::Closure{
public:
    void Run() override{
        if(m_cb != nullptr){
            m_cb();
        }
    }
private:
    std::function<void()> m_cb{nullptr};
};

}

#endif