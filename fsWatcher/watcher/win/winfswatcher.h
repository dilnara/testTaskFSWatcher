#ifndef WINFSWATCHER_H
#define WINFSWATCHER_H

#include "../fswatcher.h"

#include <thread>

class WinFSWatcher : public IFSWatcher {
private:
    IFSListener *m_fsListener;
    std::thread *m_th;

private:
    void terminateWatching();
    void indexFilesInDirectory(const std::wstring & path);
    void watchFolder(const std::wstring & path);

public:
    WinFSWatcher();
    ~WinFSWatcher();

    //IFSWatcher interface
    void startWatch(const std::wstring & path);
    void stopWatch();
    void setListener(IFSListener *);
    bool renameFile(const std::wstring & from, const std::wstring & to);
};

#endif // WINFSWATCHER_H
