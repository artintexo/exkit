#pragma once

#include "base/base.h"

class DataSim;
class ExecSim;

class Simulator {
public:
    Simulator(Holder *hld);
    ~Simulator();

private:
    Holder *hld;
    DataSim *data;
    ExecSim *exec;
};
