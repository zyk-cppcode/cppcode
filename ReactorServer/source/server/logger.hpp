#pragma once

#include <iostream>
#include <string>
#include <filesystem> 
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <memory>
#include <sstream>
#include "Mutex.hpp"

// 规定日志等级
enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

inline std::string Level2String(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "Debug";
    case LogLevel::INFO:
        return "Info";
    case LogLevel::WARNING:
        return "Warning";
    case LogLevel::ERROR:
        return "Error";
    case LogLevel::FATAL:
        return "Fatal";
    default:
        return "Unknown";
    }
}

// 20XX-08-04 12:27:03
inline std::string GetCurrentTime()
{
    // 1. 获取时间戳
    time_t currtime = time(nullptr);

    // 2. 如何把时间戳转换成为20XX-08-04 12:27:03
    struct tm currtm;
    localtime_r(&currtime, &currtm);

    // 3. 转换成为字符串 -- dubug?
    char timebuffer[64];
    snprintf(timebuffer, sizeof(timebuffer), "%4d-%02d-%02d %02d:%02d:%02d",
             currtm.tm_year + 1900,
             currtm.tm_mon + 1,
             currtm.tm_mday,
             currtm.tm_hour,
             currtm.tm_min,
             currtm.tm_sec);

    return timebuffer;
}


class LogStrategy
{
public:
    virtual ~LogStrategy() = default;
    virtual void SyncLog(const std::string &logmessage) = 0;
};

// 显示器刷新
class ConsoleLogStrategy : public LogStrategy
{
public:
    ~ConsoleLogStrategy()
    {
    }
    void SyncLog(const std::string &logmessage) override
    {
        {
            LockGuard lockguard(&_lock);
            std::cout << logmessage << std::endl;
        }
    }

private:
    Mutex _lock;
};

const std::string logdefaultdir = "log";
const static std::string logfilename = "test.log";

// 文件刷新
class FileLogStrategy : public LogStrategy
{
public:
    FileLogStrategy(const std::string &dir = logdefaultdir,
                    const std::string filename = logfilename)
        : _dir_path_name(dir), _filename(filename)
    {
        LockGuard lockguard(&_lock);
        if (std::filesystem::exists(_dir_path_name))
        {
            return;
        }
        try
        {
            std::filesystem::create_directories(_dir_path_name);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << e.what() << "\r\n";
        }
    }
    void SyncLog(const std::string &logmessage) override
    {
        {
            LockGuard lockguard(&_lock);
            std::string target = _dir_path_name;
            target += "/";
            target += _filename;

            std::ofstream out(target.c_str(), std::ios::app); // append
            if (!out.is_open())
            {
                return;
            }
            out << logmessage << "\n"; // out.write
            out.close();
        }
    }

    ~FileLogStrategy()
    {
    }

private:
    std::string _dir_path_name; // log
    std::string _filename;      // hello.log => log/hello.log
    Mutex _lock;
};


class Logger
{
public:
    Logger()
    {
    }
    void EnableConsoleLogStrategy()
    {
        _strategy = std::make_unique<ConsoleLogStrategy>();
    }
    void EnableFileLogStrategy()
    {
        _strategy = std::make_unique<FileLogStrategy>();
    }
    // 形成一条完整日志
    class LogMessage
    {
    public:
        LogMessage(LogLevel level, std::string &filename, int line, Logger &logger)
        : _curr_time(GetCurrentTime()),
          _level(level),
          _pid(getpid()),
          _filename(filename),
          _line(line),
          _logger(logger)
        {
            std::stringstream ss;
            ss << "[" << _curr_time << "] "
               << "[" << Level2String(_level) << "] "
               << "[" << _pid << "] "
               << "[" << _filename << "] "
               << "[" << _line << "]"
               <<  " >>> ";
            _loginfo = ss.str();
        }
        template<typename T>
        LogMessage& operator << (const T &info)
        {
            std::stringstream ss;
            ss << info;
            _loginfo += ss.str();
            return *this;
        }

        ~LogMessage()
        {
            if(_logger._strategy)
            {
                _logger._strategy->SyncLog(_loginfo);
            }
        }
    private:
        std::string _curr_time; // 日志时间
        LogLevel _level; // 日志等级
        pid_t _pid; // 进程pid
        std::string _filename;
        int _line;

        std::string _loginfo; // 一条完整的日志
        Logger &_logger; // 提供刷新策略的具体做法
    };
    LogMessage operator()(LogLevel level, std::string filename, int line)
    {
        return LogMessage(level, filename, line, *this);
    } 
    ~Logger()
    {
    }

private:
    std::unique_ptr<LogStrategy> _strategy;
};

inline Logger logger;

#define LOG(level) logger(level, __FILE__, __LINE__)
#define EnableConsoleLogStrategy() logger.EnableConsoleLogStrategy()
#define EnableFileLogStrategy() logger.EnableFileLogStrategy()