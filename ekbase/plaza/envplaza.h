#pragma once

#include "plaza.h"

class EnvPlaza {
public:
    EnvPlaza(Plaza *plz);
    ~EnvPlaza();

private:
    Plaza *plz;
    Holder *hld;
};
