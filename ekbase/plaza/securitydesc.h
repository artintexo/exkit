#pragma once

#include "description.h"

class SecurityDesc : public Description {
public:
    SecurityDesc();

    size_t replId;
    size_t replRev;
    size_t replAct;

    size_t isin;
    size_t sessId;
    size_t isinId;
    size_t state;

    size_t minStep;
    size_t roundto;

    size_t lastClQuote;
    size_t limitUp;
    size_t limitDown;
};
