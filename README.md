###日志模块
```
1.日志等级
2.打印到文件，支持日期命名，支持日志的滚动
3.c格式化风控
4.线程安全
```

LogLevel
```
Debug
Info
Error
```
LogEvent
```
文件名、行号
Msgno
进程号
Thread id 
日期时间，精确到毫秒
自定义信息
```

日志格式
```
[level][%y-%m-%d %H:%M:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg]
```

Logger 日志器
```
1.提供打印日志方法
2.设置日志输出路径
```
