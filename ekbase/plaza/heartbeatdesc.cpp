#include "heartbeatdesc.h"

HeartbeatDesc::HeartbeatDesc()
    : Description("heartbeat")
{
    Push(FieldDesc("replID", "i8", &replId));
    Push(FieldDesc("replRev", "i8", &replRev));
    Push(FieldDesc("replAct", "i8", &replAct));

    Push(FieldDesc("server_time", "t", &serverTime));
}
