#include "winutils.h"

#include <stack>

#define WINDOWS_TICK 10000000ULL
#define SEC_TO_UNIX_EPOCH 11644473600ULL

LONGLONG largeIntToLong(DWORD low, DWORD high) {
    LARGE_INTEGER sz;
    sz.LowPart = low;
    sz.HighPart = high;
    return sz.QuadPart;
}

time_t filetimeToTime_t(FILETIME ft) {
    return largeIntToLong(ft.dwLowDateTime, ft.dwHighDateTime) / WINDOWS_TICK - SEC_TO_UNIX_EPOCH;
}

FSFileInfo getFileInfoByPath(const std::wstring & path) {
    _WIN32_FILE_ATTRIBUTE_DATA fattrs;
    if (::GetFileAttributesEx(
                path.c_str(),
                ::GetFileExInfoStandard,
                &fattrs) == 0)
        return FSFileInfo(path);

    return FSFileInfo(
                path,
                largeIntToLong(fattrs.nFileSizeLow, fattrs.nFileSizeHigh),
                filetimeToTime_t(fattrs.ftLastWriteTime)
           );
}

//note, that this function ignores errors when accessing files
//it reads all the files it can read.
void listFiles(const std::wstring & dirPath, std::list<FSFileInfo> & files) {
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA ffd;
    std::stack<FSFileInfo> directories;
    FSFileInfo stackedDirectory;

    directories.push(FSFileInfo(dirPath));
    files.clear();

    while (!directories.empty()) {
        stackedDirectory = directories.top();
        directories.pop();
        if (stackedDirectory.name != dirPath) {
            files.push_back(
                FSFileInfo(stackedDirectory.name, stackedDirectory.size, stackedDirectory.modified)
            );
        }

        std::wstring patternAll = stackedDirectory.name + L"\\*";
        hFind = FindFirstFileW(patternAll.c_str(), &ffd);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (wcscmp(ffd.cFileName, L".") != 0 &&
                    wcscmp(ffd.cFileName, L"..") != 0) {
                    FSFileInfo foundFileInfo = FSFileInfo(
                        stackedDirectory.name + L"\\" + ffd.cFileName,
                        largeIntToLong(ffd.nFileSizeLow, ffd.nFileSizeHigh),
                        filetimeToTime_t(ffd.ftLastWriteTime)
                    );

                    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        directories.push(foundFileInfo);
                    else
                        files.push_back(foundFileInfo);
                }
            } while(FindNextFile(hFind, &ffd) != 0);
        }
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }
}
