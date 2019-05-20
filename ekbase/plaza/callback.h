#pragma once

#include "cgate.h"

CG_RESULT ReplCB(cg_conn_t *conn, cg_listener_t *lsn, struct cg_msg_t *msg, void *data);

CG_RESULT MQReplyCB(cg_conn_t *conn, cg_listener_t *lsn, struct cg_msg_t *msg, void *data);
