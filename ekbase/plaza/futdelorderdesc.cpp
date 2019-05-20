#include "futdelorderdesc.h"

FutDelOrderDesc::FutDelOrderDesc()
    : Description("FutDelOrder")
{
    Push(FieldDesc("broker_code", "c4", &brokerCode));
    Push(FieldDesc("order_id", "i8", &orderId));
}
