#pragma once

#include "description.h"

class FutAddOrderDesc : public Description {
public:
    FutAddOrderDesc();

    size_t clientId;
    size_t brokerCode;
    size_t clientCode;
    size_t isin;
    size_t dir;
    size_t type;
    size_t price;
    size_t qty;
};
