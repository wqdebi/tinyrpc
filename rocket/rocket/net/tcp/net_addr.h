#ifndef ROCKER_NET_TCP_NET_ADDR_H
#define ROCKER_NET_TCP_NET_ADDR_H

#include<arpa/inet.h>
#include<netinet/in.h>
#include<string>
#include<memory>

namespace rocket{

class NetAddr{
public:
    typedef std::shared_ptr<NetAddr> s_ptr;
    virtual sockaddr* getSockAddr() = 0;
    virtual socklen_t getSockLen() = 0;
    virtual int getFamliy() = 0;
    virtual std::string toString() = 0;
    virtual bool checkValid() = 0;
};

class IPNetAddr: public NetAddr{
public:
    IPNetAddr(const std::string& ip, uint16_t port);
    IPNetAddr(const std::string& addr);
    IPNetAddr(sockaddr_in addr);
    sockaddr* getSockAddr();
    socklen_t getSockLen();
    int getFamliy();
    std::string toString();
    bool checkValid();
private:
    std::string m_ip;
    uint16_t m_port{0};
    sockaddr_in m_addr;
};

}

#endif