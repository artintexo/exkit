#include "securitydesc.h"

SecurityDesc::SecurityDesc()
    : Description("fut_sess_contents")
{
    Push(FieldDesc("replID", "i8", &replId));
    Push(FieldDesc("replRev", "i8", &replRev));
    Push(FieldDesc("replAct", "i8", &replAct));

    Push(FieldDesc("isin", "c25", &isin));
    Push(FieldDesc("sess_id", "i4", &sessId));
    Push(FieldDesc("isin_id", "i4", &isinId));
    Push(FieldDesc("state", "i4", &state));

    Push(FieldDesc("min_step", "d16.5", &minStep));
    Push(FieldDesc("roundto", "i4", &roundto));

    Push(FieldDesc("last_cl_quote", "d16.5", &lastClQuote));
    Push(FieldDesc("limit_up", "d16.5", &limitUp));
    Push(FieldDesc("limit_down", "d16.5", &limitDown));
}
