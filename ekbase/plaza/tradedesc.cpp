#include "tradedesc.h"

TradeDesc::TradeDesc()
    : Description("deal")
{
    Push(FieldDesc("replID", "i8", &replId));
    Push(FieldDesc("replRev", "i8", &replRev));
    Push(FieldDesc("replAct", "i8", &replAct));

    Push(FieldDesc("nosystem", "i1", &nosystem));
    Push(FieldDesc("moment_ns", "u8", &moment));
    Push(FieldDesc("isin_id", "i4", &isinId));
    Push(FieldDesc("price", "d16.5", &price));
    Push(FieldDesc("xamount", "i8", &qty));
}
