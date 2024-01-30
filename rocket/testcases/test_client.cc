#include "../rocket/common/log.h"
#include<pthread.h>
#include"../rocket/common/config.h"
#include<iostream>
#include"rocket/common/log.h"
#include<fcntl.h>
#include<assert.h>
#include<sys/socket.h>
#include<string.h>
#include<memory>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string>
#include<unistd.h>
#include"rocket/common/log.h"
#include"rocket/net/tcp/tcp_client.h"
#include"../rocket/common/log.h"
#include"../rocket/net/tcp/net_addr.h"
#include<iostream>
#include"rocket/net/tcp/tcp_server.h"
#include<memory>
#include"rocket/net/string_coder.h"
#include"rocket/net/abstract_protocol.h"

void test_connect(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        ERRORLOG("invalid listenfd %d", fd);
        exit(0); 
    }
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

    std::string msg = "hello world";
    rt = write(fd, msg.c_str(), msg.length());
    DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());
    char buf[100];
    rt = read(fd, buf, 100);
    DEBUGLOG("success read %d bytes, [%s]",rt, std::string(buf).c_str());
}
void test_tcp_client(){
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12345);
    rocket::TcpClient client(addr);
    client.connect([addr, &client](){
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        std::shared_ptr<rocket::StringProtocol> message = std::make_shared<rocket::StringProtocol>();
        message->info = "Hello Rocker";
        std::string a = std::string("123456");
        message->setReqId(a);
        client.writeMessage(message, [](rocket::AbstractProtocol::s_ptr msg_ptr){
            DEBUGLOG("send message success");
        });
        client.readMessage("123456", [](rocket::AbstractProtocol::s_ptr msg_ptr){
            std::shared_ptr<rocket::StringProtocol> message = std::dynamic_pointer_cast<rocket::StringProtocol>(msg_ptr);
            DEBUGLOG("req_id[%s], get response %s",message->getReqId().c_str(), message->info.c_str());
        });
    });
}
int main()
{
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    test_tcp_client();
    return 0;
    
}