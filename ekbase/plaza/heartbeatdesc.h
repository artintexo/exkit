#pragma once

#include "description.h"

class HeartbeatDesc : public Description {
public:
    HeartbeatDesc();

    size_t replId;
    size_t replRev;
    size_t replAct;

    size_t serverTime;
};
