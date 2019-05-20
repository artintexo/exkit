#include "msg101desc.h"

Msg101Desc::Msg101Desc()
    : Description("FORTS_MSG101")
{
    Push(FieldDesc("code", "i4", &code));
    Push(FieldDesc("message", "c255", &message));
    Push(FieldDesc("order_id", "i8", &orderId));
}
