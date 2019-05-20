#pragma once

#include "description.h"

class Msg103Desc : public Description {
public:
    Msg103Desc();

    size_t code;
    size_t message;
    size_t numOrders;
};
