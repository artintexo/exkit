#pragma once

#include "cgate.h"

#include <string>

#define P2DBG(res)                                                                               \
    {                                                                                            \
        if (res != CG_ERR_OK) {                                                                  \
            hld->logman->Debug(GetSystemTime(), "got 0x%X at (%s:%d)", res, __FILE__, __LINE__); \
        }                                                                                        \
    }

#define P2ERR(res)                                                                               \
    {                                                                                            \
        if (res != CG_ERR_OK) {                                                                  \
            hld->logman->Error(GetSystemTime(), "got 0x%X at (%s:%d)", res, __FILE__, __LINE__); \
            throw std::runtime_error("Plaza ERROR");                                             \
        }                                                                                        \
    }

long Convert(cg_time_t *time);
std::string Readstr(const char *type, const void *data);
double Readbcd(const void *data);
