#include "logmanager.h"
#include "datetime.h"
#include "holder.h"

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <sstream>

LogManager::LogManager(Holder *hld)
    : hld(hld)
{
    Datetime dt(GetSystemTime());
    char buf[32];
    snprintf(buf, 32, "%04d%02d%02d", dt.year, dt.month, dt.day);

    std::ostringstream oss;
    oss << "/tmp/" << hld->config.main.name << "_" << buf << ".log";

    file = std::ofstream(oss.str());
}

LogManager::~LogManager()
{
}

void LogManager::Debug(long timestamp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Debug, timestamp, fmt, args);
    va_end(args);
}

void LogManager::Info(long timestamp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Info, timestamp, fmt, args);
    va_end(args);
}

void LogManager::Warn(long timestamp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Warn, timestamp, fmt, args);
    va_end(args);
}

void LogManager::Error(long timestamp, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::Error, timestamp, fmt, args);
    va_end(args);
}

void LogManager::Log(LogLevel level, long timestamp, const char *fmt, va_list args)
{
    std::lock_guard<std::mutex> lock(mtx);

    char msg[1024];
    vsnprintf(msg, 1024, fmt, args);

    Datetime dt(timestamp);
    char buf[32];
    snprintf(buf, 32, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec, dt.ns / 1000000);

    std::ostringstream tofile;
    tofile << buf << ": " << desc[(int)level].level << msg;

    std::ostringstream tocerr;
    tocerr << buf << ": " << desc[(int)level].before << msg << desc[(int)level].after;

    using std::cerr;
    switch (level) {
    case LogLevel::Debug:
        file << tofile.str() << "\n";
        cerr << tocerr.str() << "\n"; //TODO: comment this line in release
        break;
    case LogLevel::Info:
        file << tofile.str() << "\n";
        cerr << tocerr.str() << "\n";
        break;
    case LogLevel::Warn:
    case LogLevel::Error:
        file << tofile.str() << std::endl;
        cerr << tocerr.str() << std::endl;
        break;
    }
}
