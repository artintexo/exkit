#include "envplaza.h"
#include "utils.h"

#include "base/holder.h"

#include <sstream>

EnvPlaza::EnvPlaza(Plaza *plz)
    : plz(plz), hld(plz->GetHolder())
{
    if (hld->config.main.mode == Mode::Backtest)
        throw std::runtime_error("Create EnvPlaza in Backtest mode");

    std::ostringstream oss;
    oss << "subsystems=" << hld->config.plaza.subs << ";";
    oss << "key=" << hld->config.plaza.key << ";";
    oss << "ini=" << hld->config.plaza.ini << ";";
    LOGD("%s", oss.str().c_str());

    P2ERR(cg_env_open(oss.str().c_str()));
    LOGD("EnvPlaza created");
}

EnvPlaza::~EnvPlaza()
{
    P2DBG(cg_env_close());
    LOGD("EnvPlaza destroyed");
}
