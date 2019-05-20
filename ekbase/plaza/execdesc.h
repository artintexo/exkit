#pragma once

#include "description.h"

class ExecDesc : public Description {
public:
    ExecDesc();

    size_t replId;
    size_t replRev;
    size_t replAct;

    size_t clientId;
    size_t orderId;
    size_t isinId;
    size_t dir;
    size_t fillPrice;
    size_t fillQty;
    size_t action;
    size_t status;
    size_t leavesQty;
};

/*
replID i8 Служебное поле подсистемы репликации
replRev i8 Служебное поле подсистемы репликации
replAct i8 Служебное поле подсистемы репликации

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
