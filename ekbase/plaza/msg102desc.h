#pragma once

#include "description.h"

class Msg102Desc : public Description {
public:
    Msg102Desc();

    size_t code;
    size_t message;
    size_t amount;
};
