#ifndef FSFILEINFO_H
#define FSFILEINFO_H

#include <string>

struct FSFileInfo {
    std::wstring name;
    long long size;
    time_t modified; //time_t

    FSFileInfo(const std::wstring & name = L"", long long size = 0, time_t modified = 0) :
        name(name), size(size), modified(modified) {
    }

    operator==(const FSFileInfo& _other) {
        return this->name == _other.name;
    }
};

#endif // FSFILEINFO_H
