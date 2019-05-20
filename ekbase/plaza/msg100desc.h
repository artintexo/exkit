#pragma once

#include "description.h"

class Msg100Desc : public Description {
public:
    Msg100Desc();

    size_t code;
    size_t message;
};
