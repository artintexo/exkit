#pragma once

#include "description.h"

class Msg101Desc : public Description {
public:
    Msg101Desc();

    size_t code;
    size_t message;
    size_t orderId;
};
