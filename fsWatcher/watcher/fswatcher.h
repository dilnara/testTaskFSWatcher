#ifndef FSWATCHERMODEL_H
#define FSWATCHERMODEL_H

#include "fsfileinfo.h"

#include <string>
#include <list>

class IFSListener {
public:
    virtual void onFilesListed(const std::list<FSFileInfo> & filesList) = 0;
    virtual void onAdd(const FSFileInfo &) = 0;
    virtual void onRemove(const std::wstring &) = 0;
    virtual void onModified(const FSFileInfo &) = 0;
    virtual void onRenamed(const std::wstring &, const std::wstring &) = 0;
    virtual void onError(const std::wstring &) = 0;
    virtual ~IFSListener() {}
};

class IFSWatcher {
public:
  virtual void startWatch(const std::wstring & path) = 0;
  virtual void stopWatch() = 0;
  virtual void setListener(IFSListener *) = 0;
  virtual bool renameFile(const std::wstring & from, const std::wstring & to) = 0;
  virtual ~IFSWatcher() {}
};

IFSWatcher * createFSWatcher();

#endif // FSWATCHERMODEL_H
