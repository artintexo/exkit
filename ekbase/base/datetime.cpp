#include "datetime.h"

#include <cassert>
#include <chrono>

constexpr long NS_PER_SEC = 1000000000;

Datetime::Datetime()
    : Datetime(0)
{
}

Datetime::Datetime(long timestamp)
    : timestamp(timestamp)
{
    assert(timestamp >= 0);
    long secs = timestamp / NS_PER_SEC;
    tm *local = localtime(&secs);

    year = local->tm_year + 1900;
    month = local->tm_mon + 1;
    day = local->tm_mday;
    hour = local->tm_hour;
    min = local->tm_min;
    sec = local->tm_sec;
    ns = timestamp - secs * NS_PER_SEC;

    assert(1970 <= year && year < 2038);
    assert(1 <= month && month <= 12);
    assert(1 <= day && day <= 31);
}

Datetime::Datetime(int year, int month, int day, int hour, int min, int sec, long ns)
    : year(year), month(month), day(day),
      hour(hour), min(min), sec(sec), ns(ns)
{
    assert(1970 <= year && year < 2038);
    assert(1 <= month && month <= 12);
    assert(1 <= day && day <= 31);

    tm local;
    local.tm_year = year - 1900;
    local.tm_mon = month - 1;
    local.tm_mday = day;
    local.tm_hour = hour;
    local.tm_min = min;
    local.tm_sec = sec;
    local.tm_isdst = 0;

    timestamp = mktime(&local) * NS_PER_SEC + ns;
    assert(timestamp >= 0);
}

bool Datetime::operator==(const Datetime &other)
{
    return timestamp == other.timestamp;
}

bool Datetime::operator!=(const Datetime &other)
{
    return timestamp != other.timestamp;
}

bool Datetime::operator<(const Datetime &other)
{
    return timestamp < other.timestamp;
}

bool Datetime::operator>(const Datetime &other)
{
    return timestamp > other.timestamp;
}
