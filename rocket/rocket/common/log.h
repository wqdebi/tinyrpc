#ifndef ROCKET_COMMEN_LOG_H
#define ROCKET_COMMEN_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <semaphore.h>
#include "config.h"
#include "mutex.h"

#define DEBUGLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Debug) \
  { \
    rocket::Logger::GetGlobalLogger()->pushLog((rocket::LogEvent(rocket::Loglevel::Debug)).toString()\
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log();\
  } \


#define INFOLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Info) \
  { \
    rocket::Logger::GetGlobalLogger()->pushLog((rocket::LogEvent(rocket::Loglevel::Info)).toString()\
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log();\
  } \

#define ERRORLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Error) \
  { \
    rocket::Logger::GetGlobalLogger()->pushLog((rocket::LogEvent(rocket::Loglevel::Error)).toString()\
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log();\
  } \

namespace rocket{



template<typename... Args>
std::string formatString(const char* str, Args&&... args){
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
Loglevel StringToLogLevel(const std::string& log_level);
class Logger{
public:
    typedef std::shared_ptr<Logger> s_ptr;
    Logger(Loglevel level): m_set_level(level){}
    void pushLog(const std::string& msg);
    static Logger* GetGlobalLogger();
    void log();
    Loglevel getLogLevel()const{
        return m_set_level; 
    }
    static void InitGlobalLogger();
private:
    Loglevel m_set_level;
    std::queue<std::string> m_buffer;
    Mutex m_mutex;
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
    LogEvent(Loglevel level):m_level(level){}
private:
    std::string m_file_name; //文件名
    int32_t m_file_line; //行号
    int32_t m_pid; //进程号
    int32_t m_thread_id; //线程号
    Loglevel m_level;//日志等级
};
}
#endif