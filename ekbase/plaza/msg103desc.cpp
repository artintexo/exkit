#include "msg103desc.h"

Msg103Desc::Msg103Desc()
    : Description("FORTS_MSG103")
{
    Push(FieldDesc("code", "i4", &code));
    Push(FieldDesc("message", "c255", &message));
    Push(FieldDesc("num_orders", "i4", &numOrders));
}
