#ifndef ROCKET_COMMEN_LOG_H
#define ROCKET_COMMEN_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <semaphore.h>


#define DEBUGLOG(str, ...)\
    std::string msg = (new rocket::LogEvent(rocket::Loglevel::Debug)->toString()) + rocket::formatString(str, ##__VA_ARGS__);\
    rocket::Logger::GetGlobalLogger()->pushLog(msg);\
    recket::Logger::GetGlobalLogger()->log();

namespace rocket{



template<typename... Args>
std::string formatString(const char* str, Args... args){
    int size = snprintf(nullptr, 0, str, args...);
    std::string output;
    if(size > 0){
        output.resize(size);
        snprintf(&output[0], size + 1, str, args...);
    }
    return output;
}

enum Loglevel{
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
};

std::string LogLevelToString(Loglevel level);

class Logger{
public:
    typedef std::shared_ptr<Logger> s_ptr;
    void pushLog(const std::string& msg);
    static Logger* GetGlobalLogger();
    void log();
private:
    Loglevel m_set_level;
    std::queue<std::string> m_buffer;
};


class LogEvent{
public:
    std::string getFileName()const{
        return m_file_name;
    }
    Loglevel getLogLevel() const{
        return m_level;
    }
    std::string toString();
private:
    std::string m_file_name; //文件名
    int32_t m_file_line; //行号
    int32_t m_pid; //进程号
    int32_t m_thread_id; //线程号
    Loglevel m_level;//日志等级
};



}

#endif