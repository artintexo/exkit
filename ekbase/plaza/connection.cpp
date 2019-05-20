#include "connection.h"
#include "utils.h"

#include "base/context.h"
#include "base/holder.h"

#include <thread>

Connection::Connection(Plaza *plz, std::string name, std::string settings)
    : plz(plz), hld(plz->GetHolder()), name(name), settings(settings), state(0)
{
    P2ERR(cg_conn_new(settings.c_str(), &conn));
}

Connection::~Connection()
{
    P2DBG(cg_conn_destroy(conn));
}

void Connection::Open()
{
    while (state != CG_STATE_ACTIVE && Application::GetMainFlag())
        TryOpen();
}

void Connection::Close()
{
    while (state != CG_STATE_CLOSED)
        TryClose();
}

void Connection::Process()
{
    P2ERR(cg_conn_getstate(conn, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        switch (cg_conn_process(conn, 0, 0)) {
        case CG_ERR_OK:
            break;
        case CG_ERR_TIMEOUT:
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            break;
        case CG_ERR_INVALIDARGUMENT:
            P2ERR(CG_ERR_INVALIDARGUMENT);
            break;
        case CG_ERR_INTERNAL:
            P2ERR(CG_ERR_INTERNAL);
            break;
        }
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_conn_close(conn));
        break;
    case CG_STATE_CLOSED:
        P2DBG(cg_conn_open(conn, 0));
        break;
    case CG_STATE_OPENING:
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}

void Connection::TryOpen()
{
    P2ERR(cg_conn_getstate(conn, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        LogState();
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_conn_close(conn));
        break;
    case CG_STATE_CLOSED:
        P2DBG(cg_conn_open(conn, 0));
        break;
    case CG_STATE_OPENING:
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}

void Connection::TryClose()
{
    P2ERR(cg_conn_getstate(conn, &state));

    switch (state) {
    case CG_STATE_ACTIVE:
        P2DBG(cg_conn_close(conn));
        break;
    case CG_STATE_ERROR:
        P2DBG(cg_conn_close(conn));
        break;
    case CG_STATE_CLOSED:
        LogState();
        break;
    case CG_STATE_OPENING:
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}

void Connection::LogState()
{
    switch (state) {
    case CG_STATE_ACTIVE:
        LOGD("%s ACTIVE", name.c_str());
        break;
    case CG_STATE_ERROR:
        LOGD("%s ERROR", name.c_str());
        break;
    case CG_STATE_CLOSED:
        LOGD("%s CLOSED", name.c_str());
        break;
    case CG_STATE_OPENING:
        LOGD("%s OPENING", name.c_str());
        break;
    default:
        LOGW("%s UNKNOWN 0x%X", name.c_str(), state);
        break;
    }
}
