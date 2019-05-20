#pragma once

#include "description.h"

class Msg99Desc : public Description {
public:
    Msg99Desc();

    size_t queueSize;
    size_t penaltyRemain;
    size_t message;
};
