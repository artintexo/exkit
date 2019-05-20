#include "utils.h"

#include "base/datetime.h"

//
long Convert(cg_time_t *time)
{
    Datetime dt(time->year, time->month, time->day,
                time->hour, time->minute, time->second,
                time->msec * 1000000);
    return dt.timestamp;
}

std::string Readstr(const char *type, const void *data)
{
    char buf[1024];
    size_t size = 1024;

    CG_RESULT res = cg_getstr(type, data, buf, &size);
    if (res == CG_ERR_OK)
        return buf;
    else
        return "";
}

double Readbcd(const void *data)
{
    int64_t intpart;
    int8_t scale;

    CG_RESULT res = cg_bcd_get(data, &intpart, &scale);
    if (res != CG_ERR_OK)
        return 0;

    switch (scale) {
    case 0:
        return intpart;
    case 1:
        return intpart / 10.0;
    case 2:
        return intpart / 100.0;
    case 3:
        return intpart / 1000.0;
    case 4:
        return intpart / 10000.0;
    case 5:
        return intpart / 100000.0;
    default:
        return 0;
    }
}
