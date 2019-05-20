#include "msg100desc.h"

Msg100Desc::Msg100Desc()
    : Description("FORTS_MSG100")
{
    Push(FieldDesc("code", "i4", &code));
    Push(FieldDesc("message", "c255", &message));
}
