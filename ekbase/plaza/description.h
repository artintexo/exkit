#pragma once

#include "cgate.h"

#include <string>
#include <vector>

class FieldDesc {
public:
    FieldDesc(std::string name, std::string type, size_t *offset)
        : name(name), type(type), offset(offset) {}

    std::string name;
    std::string type;
    size_t *offset;
};

class Description {
public:
    Description(std::string name);
    virtual ~Description();

    void Init(long idx, cg_field_desc_t *fdesc);
    void Deinit();

    const std::string tableName;
    long tableIdx;
    long tableRev;

protected:
    void Push(FieldDesc fdesc);

private:
    std::vector<FieldDesc> descs;
};
