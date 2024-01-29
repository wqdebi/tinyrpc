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

int main()
{
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    test_connect();
    return 0;
    
}