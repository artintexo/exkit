#pragma once

#include "base.h"

class Application {
public:
    inline static bool GetMainFlag() { return mainFlag; }
    inline static bool GetProcessFlag() { return processFlag; }

    inline static void SetMainFlag(bool value) { mainFlag = value; }
    inline static void SetProcessFlag(bool value) { processFlag = value; }

private:
    static bool mainFlag;
    static bool processFlag;
};

class Context {
public:
    Context(Config &config, IStrategy *strategy);
    ~Context();
    void Run();
    Statistics GetStatistics();

private:
    void Open();
    void Close();
    void Start();
    void Stop();
    Holder *hld;
};
