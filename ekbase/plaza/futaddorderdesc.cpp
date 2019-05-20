#include "futaddorderdesc.h"

FutAddOrderDesc::FutAddOrderDesc()
    : Description("FutAddOrder")
{
    Push(FieldDesc("ext_id", "i4", &clientId));
    Push(FieldDesc("broker_code", "c4", &brokerCode));
    Push(FieldDesc("client_code", "c3", &clientCode));
    Push(FieldDesc("isin", "c25", &isin));
    Push(FieldDesc("dir", "i4", &dir));
    Push(FieldDesc("type", "i4", &type));
    Push(FieldDesc("price", "c17", &price));
    Push(FieldDesc("amount", "i4", &qty));
}
