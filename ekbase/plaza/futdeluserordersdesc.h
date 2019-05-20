#pragma once

#include "description.h"

class FutDelUserOrdersDesc : public Description {
public:
    FutDelUserOrdersDesc();

    size_t brokerCode;
    size_t clientCode;
    size_t isin;
    size_t buySell;
};
