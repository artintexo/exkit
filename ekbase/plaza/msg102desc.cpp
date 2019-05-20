#include "msg102desc.h"

Msg102Desc::Msg102Desc()
    : Description("FORTS_MSG102")
{
    Push(FieldDesc("code", "i4", &code));
    Push(FieldDesc("message", "c255", &message));
    Push(FieldDesc("amount", "i4", &amount));
}
