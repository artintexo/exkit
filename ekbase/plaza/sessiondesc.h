#pragma once

#include "description.h"

class SessionDesc : public Description {
public:
    SessionDesc();

    size_t replId;
    size_t replRev;
    size_t replAct;

    size_t sessId;
    size_t mainState;
    size_t interState;

    size_t eveOn;
    size_t eveBegin;
    size_t eveEnd;

    size_t monOn;
    size_t monBegin;
    size_t monEnd;

    size_t mainBegin;
    size_t mainEnd;

    size_t interBegin;
    size_t interEnd;
};
