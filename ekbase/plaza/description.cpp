#include "description.h"

#include <algorithm>
#include <stdexcept>
#include <string>

Description::Description(std::string name)
    : tableName(name)
{
    Deinit();
}

Description::~Description()
{
}

void Description::Init(long idx, cg_field_desc_t *fdesc)
{
    tableIdx = idx;
    tableRev = 0;

    size_t count = 0;
    while (fdesc != nullptr) {
        std::string name = fdesc->name;
        std::string type = fdesc->type;
        size_t offset = fdesc->offset;

        auto it = std::find_if(descs.begin(), descs.end(), [&](FieldDesc &fd) {
            return fd.name == name && fd.type == type;
        });

        if (it != descs.end()) {
            *(it->offset) = offset;
            count++;
        }

        fdesc = fdesc->next;
    }

    if (descs.size() != count)
        throw std::runtime_error(tableName + " wrong count");
}

void Description::Deinit()
{
    tableIdx = -1;
    tableRev = -1;
}

void Description::Push(FieldDesc fdesc)
{
    descs.push_back(fdesc);
}
