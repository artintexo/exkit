#pragma once

#include "base/base.h"

#include "cgate.h"

class EnvPlaza;
class DataPlaza;
class ExecPlaza;
class Connection;
class Publisher;
class Listener;
class PubRepl;
class FutTradeRepl;
class FutInfoRepl;
class FutAggrRepl;
class DealsRepl;

class Plaza {
public:
    Plaza(Holder *hld);
    ~Plaza();

    inline Holder *GetHolder() { return hld; }

private:
    Holder *hld;
    EnvPlaza *envPlaza;
    DataPlaza *dataPlaza;
    ExecPlaza *execPlaza;
};

struct ReplHeader {
    long replId;
    long replRev;
    long replAct;
};
