#include "sessiondesc.h"

SessionDesc::SessionDesc()
    : Description("session")
{
    Push(FieldDesc("replID", "i8", &replId));
    Push(FieldDesc("replRev", "i8", &replRev));
    Push(FieldDesc("replAct", "i8", &replAct));

    Push(FieldDesc("sess_id", "i4", &sessId));
    Push(FieldDesc("state", "i4", &mainState));
    Push(FieldDesc("inter_cl_state", "i4", &interState));

    Push(FieldDesc("eve_on", "i1", &eveOn));
    Push(FieldDesc("eve_begin", "t", &eveBegin));
    Push(FieldDesc("eve_end", "t", &eveEnd));

    Push(FieldDesc("mon_on", "i1", &monOn));
    Push(FieldDesc("mon_begin", "t", &monBegin));
    Push(FieldDesc("mon_end", "t", &monEnd));

    Push(FieldDesc("begin", "t", &mainBegin));
    Push(FieldDesc("end", "t", &mainEnd));

    Push(FieldDesc("inter_cl_begin", "t", &interBegin));
    Push(FieldDesc("inter_cl_end", "t", &interEnd));
}
