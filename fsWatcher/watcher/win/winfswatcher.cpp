#include "winfswatcher.h"
#include "winutils.h"

#define FILE_CHANGES_BUFFER_SIZE 1024
#define TERMINATE_EVENT_ID L"GUIDOFTERMINATEEVENT"

IFSWatcher * createFSWatcher() {
    return new WinFSWatcher();
}

WinFSWatcher::WinFSWatcher() : m_fsListener(NULL), m_th(NULL) {
}

WinFSWatcher::~WinFSWatcher() {
    stopWatch();
}

void WinFSWatcher::startWatch(const std::wstring & path) {
    stopWatch();

    m_th = new std::thread(&WinFSWatcher::watchFolder, this, path);
}

void WinFSWatcher::stopWatch() {
    if (m_th) {
        terminateWatching();
        m_th->join();
        delete m_th;
        m_th = NULL;
    }
}

void WinFSWatcher::setListener(IFSListener * _listener) {
    m_fsListener = _listener;
}

bool WinFSWatcher::renameFile(const std::wstring & from, const std::wstring & to) {
    return 0 != ::MoveFile(from.c_str(), to.c_str());
}

void WinFSWatcher::terminateWatching() {
    HANDLE terminateEvent = CreateEvent(NULL, FALSE, FALSE, TERMINATE_EVENT_ID);
    ::SetEvent(terminateEvent);
}

void WinFSWatcher::indexFilesInDirectory(const std::wstring & path) {
    std::list<FSFileInfo> fileList;
    listFiles(path, fileList);
    m_fsListener->onFilesListed(fileList);
}

void WinFSWatcher::watchFolder(const std::wstring & path) {
    HANDLE hDir = ::CreateFile(
                path.c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED ,
                NULL
            );

    if (hDir == INVALID_HANDLE_VALUE) {
        m_fsListener->onError(L"Couldn't open " + path);
        return;
    }

    //get all files in directory and notify listener
    indexFilesInDirectory(path);

    HANDLE terminateEvent = CreateEvent(NULL, FALSE, FALSE, TERMINATE_EVENT_ID);
    HANDLE fileChangedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    HANDLE hEventsToWait[2] = {terminateEvent, fileChangedEvent};

    OVERLAPPED overlapped;
    ZeroMemory(&overlapped ,sizeof(OVERLAPPED));
    overlapped.hEvent = fileChangedEvent;

    // sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH * sizeof(WCHAR)
    // it's actually maximum size of result FILE_NOTIFY_INFORMATION structure
    // note that result FILE_NOTIFY_INFORMATION structure has varying size
    // its FileName declared as WCHAR[1]
    const size_t c_maxFileNotifyInformationSize = sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH * sizeof(WCHAR);
    const size_t c_fileNotifyBufferSize = FILE_CHANGES_BUFFER_SIZE*c_maxFileNotifyInformationSize;
    BYTE* fileNotifyInfos = new BYTE[c_fileNotifyBufferSize];
    DWORD bytesReturned = 0;
    bool bContinue = true;
    std::wstring renameOldName = L"";

    while (bContinue) {
        if (::ReadDirectoryChangesW(
            hDir,
            (LPVOID)fileNotifyInfos,
            c_fileNotifyBufferSize,
            TRUE, //subtree
            FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
            &bytesReturned,
            &overlapped,
            NULL
        ) == 0) {
            m_fsListener->onError(L"Couldn't start watching directory " + path);
            break;
        }

        DWORD dwWaitState = ::WaitForMultipleObjects(2, hEventsToWait, FALSE, INFINITE);
        switch (dwWaitState) {
            case WAIT_OBJECT_0:   //terminate
                bContinue = false;
                break;
            case WAIT_OBJECT_0 + 1: // file change detected
                if (::GetOverlappedResult(hDir, &overlapped, &bytesReturned, TRUE)) {
                    /*
                        If the number of bytes transferred is zero,
                        the buffer was either too large for the system to allocate or
                        too small to provide detailed information on all the changes
                        that occurred in the directory or subtree. In this case,
                        you should compute the changes by enumerating the directory or subtree.
                    */
                    if (bytesReturned == 0) {
                        //couldn't fit all the changes, recommend to refresh and continue watching
                        m_fsListener->onError(L"It's recommended to refresh watching, possibly too many changes occured");
                        break;
                    }

                    BYTE* pByte = fileNotifyInfos;
                    PFILE_NOTIFY_INFORMATION pNotify = NULL;
                    do {
                        pNotify = (PFILE_NOTIFY_INFORMATION)pByte;
                        int wFileNameLength = pNotify->FileNameLength / sizeof(WCHAR);
                        std::wstring fileName(pNotify->FileName, wFileNameLength);
                        std::wstring fullPath = path + L"\\" + fileName;

                        if (pNotify->Action == FILE_ACTION_ADDED) {
                            m_fsListener->onAdd(getFileInfoByPath(fullPath));
                        } else if (pNotify->Action == FILE_ACTION_REMOVED) {
                            m_fsListener->onRemove(fullPath);
                        } else if (pNotify->Action == FILE_ACTION_MODIFIED) {
                            m_fsListener->onModified(getFileInfoByPath(fullPath));
                        } else if (pNotify->Action == FILE_ACTION_RENAMED_OLD_NAME) {
                            renameOldName = fullPath;
                        } else if (pNotify->Action == FILE_ACTION_RENAMED_NEW_NAME) {
                            m_fsListener->onRenamed(renameOldName, fullPath);
                        }
                        pByte += pNotify->NextEntryOffset;
                    } while (pNotify->NextEntryOffset);
                } else {//if GetOverlappedResult
                    //shouldn't be here, but who knows...
                    m_fsListener->onError(L"It's recommended to refresh watching, some error occured, stopped to watch");
                    bContinue = false;
                }
                break;
            default:
                break;
        } //switch
    } //while true

    delete[] fileNotifyInfos;
    CloseHandle(hDir);
    CloseHandle(terminateEvent);
    CloseHandle(fileChangedEvent);
}
