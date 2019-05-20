#include "plaza.h"
#include "dataplaza.h"
#include "envplaza.h"
#include "execplaza.h"

#include "base/holder.h"

Plaza::Plaza(Holder *hld)
    : hld(hld), envPlaza(nullptr), dataPlaza(nullptr), execPlaza(nullptr)
{
    if (hld->config.main.mode == Mode::Realtime) {
        envPlaza = new EnvPlaza(this);
        dataPlaza = new DataPlaza(this);
        execPlaza = new ExecPlaza(this);
        hld->dgate = dataPlaza;
        hld->egate = execPlaza;
    }

    if (hld->config.main.mode == Mode::Paper) {
        envPlaza = new EnvPlaza(this);
        dataPlaza = new DataPlaza(this);
        hld->dgate = dataPlaza;
    }
}

Plaza::~Plaza()
{
    if (execPlaza != nullptr)
        delete execPlaza;

    if (dataPlaza != nullptr)
        delete dataPlaza;

    if (envPlaza != nullptr)
        delete envPlaza;
}
