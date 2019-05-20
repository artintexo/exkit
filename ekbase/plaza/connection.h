#pragma once

#include "plaza.h"

#include <atomic>
#include <chrono>
#include <string>
#include <vector>

class Connection {
public:
    Connection(Plaza *plz, std::string name, std::string settings);
    ~Connection();

    inline bool IsOpened() { return state == CG_STATE_ACTIVE; }
    inline bool IsClosed() { return state == CG_STATE_CLOSED; }
    void Open();
    void Close();
    void Process();
    inline cg_conn_t *GetConn() { return conn; }

private:
    void TryOpen();
    void TryClose();
    void LogState();

    Plaza *plz;
    Holder *hld;
    std::string name;
    std::string settings;
    cg_conn_t *conn;
    uint32_t state;
};
