#ifndef WINUTILS_H
#define WINUTILS_H

#include "../fsfileinfo.h"

#include <windows.h>
#include <list>

LONGLONG largeIntToLong(DWORD low, DWORD high);
time_t filetimeToTime_t(FILETIME ft);
FSFileInfo getFileInfoByPath(const std::wstring & path);

//note, that this function ignores errors when accessing files
//it reads all the files it can read.
void listFiles(const std::wstring & dirPath, std::list<FSFileInfo> & files);

#endif // WINUTILS_H
