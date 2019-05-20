#include "aggrdesc.h"

AggrDesc::AggrDesc()
    : Description("orders_aggr")
{
    Push(FieldDesc("replID", "i8", &replId));
    Push(FieldDesc("replRev", "i8", &replRev));
    Push(FieldDesc("replAct", "i8", &replAct));

    Push(FieldDesc("isin_id", "i4", &isinId));
    Push(FieldDesc("price", "d16.5", &price));
    Push(FieldDesc("volume", "i8", &qty));
    Push(FieldDesc("dir", "i1", &dir));
}
