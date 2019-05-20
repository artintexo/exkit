#pragma once

#include "description.h"

class FutDelOrderDesc : public Description {
public:
    FutDelOrderDesc();

    size_t brokerCode;
    size_t orderId;
};
