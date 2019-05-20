#include "futdeluserordersdesc.h"

FutDelUserOrdersDesc::FutDelUserOrdersDesc()
    : Description("FutDelUserOrders")
{
    Push(FieldDesc("broker_code", "c4", &brokerCode));
    Push(FieldDesc("code", "c3", &clientCode));
    Push(FieldDesc("isin", "c25", &isin));
    Push(FieldDesc("buy_sell", "i4", &buySell));
}
