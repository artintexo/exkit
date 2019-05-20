#pragma once

#include "base.h"
#include "eventmanager.h"
#include "logmanager.h"
#include "ordermanager.h"
#include "positionmanager.h"
#include "securitymanager.h"
#include "sessionmanager.h"
#include "statmanager.h"
#include "subsmanager.h"
#include "timemanager.h"

#define LOGD(fmt, ...) hld->logman->Debug(GetSystemTime(), fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) hld->logman->Info(GetSystemTime(), fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) hld->logman->Warn(GetSystemTime(), fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) hld->logman->Error(GetSystemTime(), fmt, ##__VA_ARGS__)

class Holder {
public:
    Holder(Config &config, IStrategy *strategy);
    ~Holder();

    Config config;

    LogManager *logman;
    TimeManager *timeman;
    SubsManager *subsman;
    OrderManager *ordman;
    PositionManager *posman;
    StatManager *statman;
    SessionManager *sessman;
    SecurityManager *secman;
    EventManager *evman;

    Plaza *plz;
    Simulator *sim;

    IStrategy *strat;
    IDataGate *dgate;
    IExecGate *egate;
};
