#include "msg99desc.h"

Msg99Desc::Msg99Desc()
    : Description("FORTS_MSG99")
{
    Push(FieldDesc("queue_size", "i4", &queueSize));
    Push(FieldDesc("penalty_remain", "i4", &queueSize));
    Push(FieldDesc("message", "c128", &message));
}
