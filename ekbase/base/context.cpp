#include "context.h"
#include "holder.h"

#include "simulator/datasim.h"

#include <cassert>
#include <csignal>
#include <iostream>
#include <thread>

bool Application::mainFlag = true;
bool Application::processFlag = true;

Context::Context(Config &config, IStrategy *strategy)
{
    Application::SetMainFlag(true);
    Application::SetProcessFlag(true);
    hld = new Holder(config, strategy);
}

Context::~Context()
{
    delete hld;
}

static void HandleSignal(int)
{
    std::cerr << "\n";
    Application::SetMainFlag(false);
    Application::SetProcessFlag(false);
}

void Context::Run()
{
    signal(SIGINT, HandleSignal);
    LOGI("Run...");

    Open();
    Start();

    while (Application::GetProcessFlag())
        hld->evman->Process();

    Stop();
    Close();

    LOGI("Stop");
}

Statistics Context::GetStatistics()
{
    return hld->statman->GetStatistcs();
}

void Context::Open()
{
    LOGD("Gates opening...");
    hld->dgate->Open();
    hld->egate->Open();

    while (Application::GetMainFlag() && (hld->dgate->IsOpened() == false || hld->egate->IsOpened() == false))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // process session and securties
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    for (int i = 0; i < 100; i++)
        hld->evman->Process();
    //
}

void Context::Close()
{
    hld->dgate->Close();
    hld->egate->Close();

    while (hld->dgate->IsClosed() == false || hld->egate->IsClosed() == false)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    LOGD("Gates closed");
}

void Context::Start()
{
    LOGD("Context starting...");
    hld->strat->OnStart();
    hld->evman->SetStarted(true);

    if (hld->config.main.mode == Mode::Backtest) {
        DataSim *dsim = static_cast<DataSim *>(hld->dgate);
        dsim->Start();
    }
}

void Context::Stop()
{
    if (hld->config.main.mode == Mode::Backtest) {
        DataSim *dsim = static_cast<DataSim *>(hld->dgate);
        dsim->Stop();
    }

    hld->evman->SetStarted(false);
    hld->strat->OnStop();
    LOGD("Context stopped");
}
