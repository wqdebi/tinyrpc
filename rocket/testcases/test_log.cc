#include "../rocket/common/log.h"
#include<pthread.h>
#include"../rocket/common/config.h"
#include<iostream>
void* func(void *){
    int i = 5;
    while(i--){
        DEBUGLOG("this is thread in %s", "fun");
        INFOLOG("info this is thread in %s", "fun");
    }
    return NULL;
}
int main()
{
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");


    rocket::Logger::InitGlobalLogger();
    
    pthread_t thread;
    pthread_create(&thread, NULL, &func, NULL);
    int i = 5;
    while(i--){
        DEBUGLOG("test log %s", "11");
        INFOLOG("test info log %s", "11");
    }
    pthread_join(thread, NULL);
    // TiXmlDocument *xml_document = new TiXmlDocument();
    // bool rt = xml_document->LoadFile("../testcases/a.xml");
    // if(!rt){
    //     printf("Start rocket server error, failed to read config file %s\n", "a.xml");
    //     exit(0);
    // }

    return 0;
    
}
