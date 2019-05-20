#pragma once

#include "base.h"

#include <fstream>
#include <mutex>

class LogManager {
public:
    LogManager(Holder *hld);
    ~LogManager();

    void Debug(long timestamp, const char *fmt, ...);
    void Info(long timestamp, const char *fmt, ...);
    void Warn(long timestamp, const char *fmt, ...);
    void Error(long timestamp, const char *fmt, ...);
    void Log(LogLevel level, long timestamp, const char *fmt, va_list args);

private:
    struct LogLevelDesc {
        const char *level;
        const char *before;
        const char *after;
    };

    LogLevelDesc desc[4] = {
        {"[d] ", "\033[1;30m", "\033[0m"},
        {"[i] ", "\033[1;37m", "\033[0m"},
        {"[w] ", "\033[1;33m", "\033[0m"},
        {"[e] ", "\033[1;31m", "\033[0m"},
    };

    Holder *hld;
    std::mutex mtx;
    std::ofstream file;
};
