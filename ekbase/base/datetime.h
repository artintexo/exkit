#pragma once

#include <string>

class Datetime {
public:
    Datetime();
    Datetime(long timestamp);
    Datetime(int year, int month, int day, int hour = 0, int min = 0, int sec = 0, long ns = 0);

    bool operator==(const Datetime &other);
    bool operator!=(const Datetime &other);
    bool operator<(const Datetime &other);
    bool operator>(const Datetime &other);

    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int ns;
    long timestamp;
};
