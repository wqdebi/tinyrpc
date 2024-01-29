#include"log.h"
#include<sys/time.h>
#include"util.h"
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include<stdio.h>
#include "config.h"

namespace rocket{
static Logger* g_logger = nullptr;
Logger* Logger::GetGlobalLogger(){
        return g_logger;
}
void Logger::InitGlobalLogger(){
    Loglevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
    g_logger = new Logger(global_log_level);
}

std::string LogLevelToString(Loglevel level){
    switch (level)
    {
    case Debug:
        return "DEBUG";
    case Info:
        return "INFO";
    case Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}
Loglevel StringToLogLevel(const std::string& log_level){
    if(log_level == "DEBUG"){
        return Debug;
    }else if(log_level == "INFO"){
        return Info;
    }else if(log_level == "ERROR"){
        return Error;
    }else{
        return Unknown;
    }
}

std::string LogEvent::toString(){
    struct timeval now_time;
    gettimeofday(&now_time, nullptr);
    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t);
    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
    std::string time_str(buf);
    int ms = now_time.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();
    std::stringstream ss;

    ss << "[" << LogLevelToString(m_level) << "]\t"
    << "[" << time_str << "]\t"
    << "[" << m_pid << ":" << m_thread_id << "]\t";
    return ss.str();
}

void Logger::pushLog(const std::string& msg ){
    ScopeMutex<Mutex> lock(m_mutex);
    m_buffer.push(msg);
    lock.unlock();
}

void Logger::log(){
    ScopeMutex<Mutex> lock(m_mutex);
    std::queue<std::string> tmp;
    m_buffer.swap(tmp);
    lock.unlock();
    while(!tmp.empty()){
        std::string msg = tmp.front();
        tmp.pop();
        printf(msg.c_str());
    }
}

}
