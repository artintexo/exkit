#pragma once

#include "description.h"

class TradeDesc : public Description {
public:
    TradeDesc();

    size_t replId;
    size_t replRev;
    size_t replAct;

    size_t nosystem;
    size_t moment;
    size_t isinId;
    size_t price;
    size_t qty;
};
