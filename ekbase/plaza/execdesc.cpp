#include "execdesc.h"

ExecDesc::ExecDesc()
    : Description("orders_log")
{
    Push(FieldDesc("replID", "i8", &replId));
    Push(FieldDesc("replRev", "i8", &replRev));
    Push(FieldDesc("replAct", "i8", &replAct));

    Push(FieldDesc("ext_id", "i4", &clientId));
    Push(FieldDesc("id_ord", "i8", &orderId));

    Push(FieldDesc("isin_id", "i4", &isinId));
    Push(FieldDesc("dir", "i1", &dir));

    Push(FieldDesc("deal_price", "d16.5", &fillPrice));
    Push(FieldDesc("xamount", "i8", &fillQty));

    Push(FieldDesc("action", "i1", &action));
    Push(FieldDesc("xstatus", "i8", &status));
    Push(FieldDesc("xamount_rest", "i8", &leavesQty));
}

/*
ext_id i4 Внешний номер
id_ord i8 Номер заявки

isin_id i4 Уникальный числовой идентификатор инструмента
dir i1 Направление

deal_price d16.5 Цена заключенной сделки
xamount i8 Количество в операции

action i1 Действие с заявкой
xstatus i8 Расширенный статус заявки
xamount_rest i8 Оставшееся количество в заявке
*/
