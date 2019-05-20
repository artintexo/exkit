#pragma once

#include "description.h"

class AggrDesc : public Description {
public:
    AggrDesc();

    size_t replId;
    size_t replRev;
    size_t replAct;

    size_t isinId;
    size_t price;
    size_t qty;
    size_t dir;
};
