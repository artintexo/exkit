#pragma once

#include "base.h"

#include <fstream>

class DataReader {
public:
    DataReader(std::string filename);
    ~DataReader();

    Event *PeekEvent();
    void ReadNext();

private:
    Event *ReadObook(std::vector<std::string> data);
    Event *ReadTrade(std::vector<std::string> data);
    Event *ReadSession(std::vector<std::string> data);
    Event *ReadSecurity(std::vector<std::string> data);

    std::string filename;
    std::ifstream file;
    Event *event;
};
