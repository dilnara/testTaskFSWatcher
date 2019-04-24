#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "renamefiledialog.h"
#include "../watcher/fswatcher.h"

#include <QMainWindow>
#include <list>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public IFSListener {
    Q_OBJECT

signals:
    void signalOnAdd(const FSFileInfo &);
    void signalOnRemove(const std::wstring &);
    void signalOnModified(const FSFileInfo &);
    void signalOnRenamed(const std::wstring &, const std::wstring &);
    void signalOnError(const std::wstring &);
    void signalOnFilesListed(const std::list<FSFileInfo> &);

protected:
    //IFSListener interface
    void onError(const std::wstring &);
    void onAdd(const FSFileInfo &);
    void onRemove(const std::wstring &);
    void onModified(const FSFileInfo &);
    void onRenamed(const std::wstring &, const std::wstring &);
    void onFilesListed(const std::list<FSFileInfo> &);

private:
     void setTableRow(int, const FSFileInfo &);
     void startWatch();
     void updateTotal();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
     void requestDirectory();

private slots:
    void handleChoseFolderButton();
    void handleRewatchButton();
    void handleOnError(const std::wstring &);
    void handleOnAdd(const FSFileInfo &);
    void handleOnRemove(const std::wstring &);
    void handleOnModified(const FSFileInfo &);
    void handleOnRenamed(const std::wstring &, const std::wstring &);
    void handleOnFilesListed(const std::list<FSFileInfo> &);

    void handleTableClicked(const QModelIndex &);
    void handleRenameFileDialogAccepted();

private:
    Ui::MainWindow *m_ui;
    RenameFileDialog *m_rfd;
    IFSWatcher *m_fsWatcher;
    QString m_pathToWatch;
};

#endif // MAINWINDOW_H
